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
#include <libgen.h>
#include "msg.h"
#include "util.h"

#define NUM_ARGS_CLIENT 3
#define MSG_SIZE 256
#define REGION_NAME_LENGTH 16
#define MAX_LINE_LEN 4096
#define MAX_PATH_LEN 4096
#define MAX_IP_ADDR_LEN 500

int main(int argc, char** argv) {

  // Parse args
  if (argc < NUM_ARGS_CLIENT + 1) {
    printf("Usage: ./client <Request File> <Server IP> <Server Port>\n");
    exit(1);
  }
  char* server_ip = argv[2];
  int portnum = atoi(argv[3]);

  // Parse the correct basedir and filename from the given arg
  char reqfilepath[MAX_PATH_LEN];
  realpath(argv[1], reqfilepath);  // Resolve file to absolute path

  // Extract the dir of the file (creat copy since reqfilepath_copy will
  // be modified by dirname)
  char reqfilepath_copy[MAX_PATH_LEN];
  strcpy(reqfilepath_copy, reqfilepath);
  char* reqdir = dirname(reqfilepath_copy);

  // Create a TCP socket.
  int sock = socket(AF_INET , SOCK_STREAM , 0);

  // Specify an address to connect to (we use the local host or 'loop-back' address).
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(portnum);
  address.sin_addr.s_addr = inet_addr(server_ip);

  // Connect it.
  if (connect(sock, (struct sockaddr *) &address, sizeof(address)) == 0) {

    // Save server ip addr to string for easy reuse
    char server_ip_addr[MAX_IP_ADDR_LEN];
    sprintf(server_ip_addr, "%s:%d",
    inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    printf("Initiated connection with server at %s\n", server_ip_addr);

    // Open the file.
    FILE* file = fopen(reqfilepath, "r");
    if (file == NULL) {
      perror("Could not open file");
      exit(1);
    }

    // Read the file line by line and send requests to socket
    char line[MAX_LINE_LEN];
    while (fgets(line, MAX_LINE_LEN, file)) {

      // Create the request from the line
      char* cleanline = trimwhitespace(line);
      struct request_msg req;
      if (parse_req_file_line(cleanline, &req, reqdir) == 0) {
        fprintf(stderr, "Could not parse line from file\n");
        exit(1);
      }

      // Convert request struct to request string
      char req_str[MSG_SIZE];
      req_to_str(&req, req_str);

      // Send request string to server
      char region[REGION_NAME_LENGTH];
      if (strcmp(req.region_name, "") == 0) {
        sprintf(region, "(null)");
      } else {
        sprintf(region, "%s", req.region_name);
      }
      char req_data[MSG_SIZE];
      if (strcmp(req.data, "") == 0) {
        sprintf(req_data, "(null)");
      } else {
        sprintf(req_data, "%s", req.data);
      }
      printf("Sending request to server: %s %s %s\n", req.code, region, req_data);
      if (send(sock, (void*)req_str, MSG_SIZE, 0) != MSG_SIZE) {
        fprintf(stderr, "Did not send full msg\n");
        exit(1);
      }

      // Receive response string from server
      char resp_str[MSG_SIZE];
      recv(sock, (void*)resp_str, MSG_SIZE, 0);

      // Convert response string to response struct
      struct response_msg resp;
      parse_resp_msg_str(resp_str, &resp);
      char resp_data[MSG_SIZE];
      if (strcmp(resp.data, "") == 0) {
        sprintf(resp_data, "(null)");
      } else {
        sprintf(resp_data, "%s", resp.data);
      }
      printf("Received response from server: %s %s\n", resp.code, resp_data);
    }

    // Close the file and socket
    fclose(file);
    close(sock);
    printf("Closed connection with server at %s\n", server_ip_addr);
  } else {
    perror("Connection failed!");
  }
}
