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
      char* msg = (char*)malloc(msg_size * sizeof(char));
      recv(newsock, (void*)msg, msg_size, 0);

      printf("%d. %s", linenum, msg);

      free(msg);
      linenum++;
    }

    // Close the connection.
    close(newsock);
  }

  // Close the server socket.
  close(sock);
}
