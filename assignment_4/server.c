#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "msg.h"
#include "dag.h"

#define NUM_ARGS_SERVER 2
#define MAX_CONNECTIONS 100
#define MSG_SIZE 256

int main(int argc, char** argv) {

  // Parse args
  if (argc > NUM_ARGS_SERVER + 1) {
    printf("Wrong number of args, expected %d, given %d\n",
        NUM_ARGS_SERVER, argc - 1);
    exit(1);
  }
  char* dagfile = argv[1];
  int portnum = atoi(argv[2]);

  // Create a TCP socket.
  int sock = socket(AF_INET , SOCK_STREAM , 0);

  // Bind it to a local address.
  struct sockaddr_in servAddress;
  servAddress.sin_family = AF_INET;
  servAddress.sin_port = htons(portnum);
  servAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sock, (struct sockaddr *) &servAddress, sizeof(servAddress));

  // Listen on this socket.
  if (listen(sock, MAX_CONNECTIONS) < 0) {
    perror("Error on listen");
    exit(1);
  }
  printf("Server listening on port %d\n", portnum);

  // Server runs infinitely
  while (1) {
    // Accept an incoming connection
    struct sockaddr_in clientAddress;
    socklen_t size = sizeof(struct sockaddr_in);
    int newsock = accept(sock, (struct sockaddr *)&clientAddress, &size);
    if (newsock < 0) {
      perror("Error accepting connection");
      exit(1);
    }
    printf("Connection initiated from client at %s:%d\n",
        inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port);

    // Recive request
    char req_str[MSG_SIZE];
    int nbytes;
    while (nbytes = recv(newsock, (void*)&req_str, MSG_SIZE, 0) > 0) {
      printf("RECV: |%s|\n", req_str);

      // Convert request string to request struct
      struct request_msg req;
      parse_req_msg_str(req_str, &req);

      // Handle request and form the response
      struct response_msg resp;
      handle_request(&req, &resp);

      // Convert response to string message
      char resp_str[MSG_SIZE];
      resp_to_str(&resp, resp_str);

      // Then send the response string to client
      if (send(newsock, (void*)resp_str, MSG_SIZE, 0) != MSG_SIZE) {
        fprintf(stderr, "Did not send full msg\n");
        exit(1);
      }
      printf("SENT: |%s|\n", resp_str);
    }

    // Close the connection.
    close(newsock);
  }

  // Close the server socket.
  close(sock);
}
