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
#define MSG_SIZE 16

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

    // Buffer for data.
    char buf[MSG_SIZE];

    char size_str[10];
    int nbytes;
    int linenum = 0;

    // Recive size msg
    while (nbytes = recv(newsock, (void*)&size_str, 10, 0) > 0) {

      int msg_size = atoi(size_str);

      // Recive actual message
      char* msg_str = (char*)malloc(msg_size * sizeof(char));
      recv(newsock, (void*)msg_str, msg_size, 0);
      printf("RECV: |%s|\n", msg_str);

      // Convert message to request
      struct request_msg req;
      req.code = NULL;
      req.region_name = NULL;
      req.data = NULL;
      parse_request_msg_string(msg_str, &req);

      // Handle request and form the response
      struct response_msg resp;
      resp.code = NULL;
      resp.data = NULL;
      handle_request(&req, &resp);

      // Convert response to string message
      char* response_str = response_to_string(&resp);
      if (response_str == NULL) {
        fprintf(stderr, "Could not convert response to string\n");
        exit(1);
      }

      // First send mesage of how long response is to client
      char size_str[10];
      sprintf(size_str, "%d", strlen(response_str));
      if (send(newsock, (void*)size_str, 10, 0) != 10) {
        fprintf(stderr, "Did not send full msg\n");
        exit(1);
      }

      // Then send actual request
      if (send(newsock, (void*)response_str, strlen(response_str), 0) !=
          strlen(response_str)) {
        fprintf(stderr, "Did not send full msg\n");
        exit(1);
      }
      printf("SENT: |%s|\n", response_str);

      free(msg_str);
      linenum++;
    }

    // Close the connection.
    close(newsock);
  }

  // Close the server socket.
  close(sock);
}
