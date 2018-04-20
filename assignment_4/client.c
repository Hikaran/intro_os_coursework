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
#include "util.h"

#define NUM_ARGS_CLIENT 3
#define MSG_SIZE 16
#define MAX_LINE_LEN 4096

int main(int argc, char** argv) {

  // Parse args
  if (argc < NUM_ARGS_CLIENT + 1) {
    printf("Wrong number of args, expected %d, given %d\n",
        NUM_ARGS_CLIENT, argc - 1);
    exit(1);
  }
  char* reqfile = argv[1];
  char* server_ip = argv[2];
  int portnum = atoi(argv[3]);

  // Create a TCP socket.
  int sock = socket(AF_INET , SOCK_STREAM , 0);

  // Specify an address to connect to (we use the local host or 'loop-back' address).
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(portnum);
  address.sin_addr.s_addr = inet_addr(server_ip);

  // Connect it.
  if (connect(sock, (struct sockaddr *) &address, sizeof(address)) == 0) {

    // Open the file.
    FILE* file = fopen(reqfile, "r");
    if (file == NULL) {
      perror("Could not open file");
      exit(1);
    }

    // Read the file line by line and send requests to socket
    char line[MAX_LINE_LEN];
    while (fgets(line, MAX_LINE_LEN, file)) {
      char* cleanline = trimwhitespace(line);
      struct request_msg request;
      if (parse_request_file_line(cleanline, &request) == 0) {
        fprintf(stderr, "Could not parse line from file\n");
        exit(1);
      }
      char* request_str = request_to_string(&request);
      if (request_str == NULL) {
        fprintf(stderr, "Could not convert request to string\n");
        exit(1);
      }

      // First send mesage of how long request is
      char size_str[10];
      sprintf(size_str, "%d", strlen(request_str));
      if (send(sock, (void*)size_str, 10, 0) != 10) {
        fprintf(stderr, "Did not send full msg\n");
        exit(1);
      }

      // Then send actual request
      if (send(sock, (void*)request_str, strlen(request_str), 0) !=
          strlen(request_str)) {
        fprintf(stderr, "Did not send full msg\n");
        exit(1);
      }

      printf("SENT: |%s|\n", request_str);
      free_request_msg_fields(&request);
      free(request_str);
    }

    // Close the file.
    fclose(file);
    close(sock);

  } else {
    perror("Connection failed!");
  }
}
