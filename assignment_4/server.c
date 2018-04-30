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
#define MAX_IP_ADDR_LEN 500
#define MAX_LOG_MSG_LEN 2048

struct conn_args {
  int client_sock;
  struct sockaddr_in client_addr;
  pthread_mutex_t* log_mutex;
  struct dag_t* dag;
};

/** Handle the client connected to the server.
 *  The arg is a file descriptor (int) of the connected client.
 *  MUST close connection when done.
 */
void* handle_connection(void* arg) {
  struct conn_args* args = (struct conn_args*)arg;
  int client_sock = args->client_sock;
  struct sockaddr_in* client_addr = &(args->client_addr);
  pthread_mutex_t* log_mutex = args->log_mutex;
  struct dag_t* dag = args->dag;


  // Save client ip addr to string for easy reuse
  char client_ip_addr[MAX_IP_ADDR_LEN];
  sprintf(client_ip_addr, "%s:%d",
      inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));

  pthread_mutex_lock(log_mutex);
  printf("Connection initiated from client at %s\n", client_ip_addr);
  pthread_mutex_unlock(log_mutex);

  // Recive request
  char req_str[MSG_SIZE];
  int nbytes;
  while (nbytes = recv(client_sock, (void*)&req_str, MSG_SIZE, 0) > 0) {

    // Convert request string to request struct
    struct request_msg req;
    parse_req_msg_str(req_str, &req);

    pthread_mutex_lock(log_mutex);
    printf("Request received from client at %s, %s, %s\n",
        client_ip_addr, req.code, req.data);
    pthread_mutex_unlock(log_mutex);

    // Handle request and form the response
    struct response_msg resp;
    handle_request(dag, &req, &resp);

    // Convert response to string message
    char resp_str[MSG_SIZE];
    resp_to_str(&resp, resp_str);

    // Then send the response string to client
    pthread_mutex_lock(log_mutex);
    printf("Sending response to client at %s, %s %s\n",
        client_ip_addr, resp.code, resp.data);
    pthread_mutex_unlock(log_mutex);
    if (send(client_sock, (void*)resp_str, MSG_SIZE, 0) != MSG_SIZE) {
      fprintf(stderr, "Did not send full msg\n");
      exit(1);
    }
  }

  // Close connection
  close(client_sock);

  pthread_mutex_lock(log_mutex);
  printf("Closed connection with client at %s\n", client_ip_addr);
  pthread_mutex_unlock(log_mutex);

  // Free argument
  free(args);
}

int main(int argc, char** argv) {

  // Parse args
  if (argc < NUM_ARGS_SERVER + 1) {
    printf("Wrong number of args, expected %d, given %d\n",
        NUM_ARGS_SERVER, argc - 1);
    exit(1);
  }
  char* dagfilepath = argv[1];
  int portnum = atoi(argv[2]);

  // Create a log mutex to synchronise outputs
  pthread_mutex_t log_mutex;
  if (pthread_mutex_init(&log_mutex, NULL)) {
    perror("Could not inialize mutex");
    exit(1);
  }

  // Create and init dag
  struct dag_t dag;
  init_dag(&dag, dagfilepath);

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
    struct conn_args* thread_args = malloc(sizeof(struct conn_args));
    thread_args->log_mutex = &log_mutex;
    thread_args->dag = &dag;
    socklen_t size = sizeof(struct sockaddr_in);
    thread_args->client_sock = accept(
        sock, (struct sockaddr *)&(thread_args->client_addr), &size);

    if (thread_args->client_sock < 0) {
      perror("Error accepting connection");
      exit(1);
    } else {
      pthread_t conn_thread;
      if (pthread_create(&conn_thread, NULL, handle_connection, (void*)thread_args) != 0) {
        perror("Could not create thread");
      }
      if (pthread_detach(conn_thread) != 0) {
        perror("Could not detach thread");
      }
    }
  }

  // Close the server socket.
  close(sock);
}
