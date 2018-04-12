/*login: swann013, tangx254
* date: 04/11/18
* name: Kristopher Swann, Joseph Tang
* login: swann013, tangx254 */

#define _BSD_SOURCE
#define NUM_ARGS 3
#define MAX_STR_LEN 1024
#define INIT_NUM_CHILDREN 2

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>

#include "dag.h"
#include "rmrf.h"
#include "queue.h"

/**
 * Add files from directory to queue.
 *
 * Returns the number of files placed in queue.
 */
int queue_files(struct queue_t* queue, char* dir_name) {
  DIR* dir = opendir(dir_name);
  if (dir == NULL) {
    perror("Could not open input directory");
    exit(1);
  }

  struct dirent* entry;
  int num_files = 0;
  errno = 0;  // Reset errno

  while (entry = readdir(dir)) {
    // Ignore entries that do not correspond to regular files.
    if (entry->d_type != DT_REG) {
      continue;
    }
    enqueue(queue, entry->d_name);
    num_files++;
  }

  if (errno) {
    perror("Unexpected issue with readdir()");
    exit(1);
  }

  return num_files;
}

int main(int argc, char **argv) {
  if (argc != NUM_ARGS + 1) {
    printf("Incorrect number of arguments supplied.\n");
    printf("Usage: ./votecounter <DAG.txt> <input_dir> <output_dir>\n");
    exit(1);
  }

  // Build graph from input file.
  struct dag_node_t* root = parse_dag_file(argv[1], INIT_NUM_CHILDREN);

  // Remove output directory if it already exists.
  char* output_dir_name = argv[3];
  DIR* output_dir = opendir(output_dir_name);
  if (output_dir != NULL) {
    closedir(output_dir);
    rmrf(output_dir_name);
  }

  // Create output directory.
  if (mkdir(output_dir_name, 0777) && errno != EEXIST) {
    char error_msg[MAX_STR_LEN];
    sprintf(error_msg, "Could not create dir %s", output_dir_name);
    perror(error_msg);
    exit(1);
  }

  // Build directory structure from graph in output directory.
  create_dir_structure(root, output_dir_name);

  // Initialize dynamic shared queue.
  struct queue_t* file_queue = (struct queue_t*)malloc(sizeof(struct queue_t));
  init_queue(file_queue);

  // Retrieve file names from input directory.
  char* input_dir_name = argv[2];
  int num_threads = queue_files(file_queue, input_dir_name);

  printf("# of files queued: %d\n", num_threads);
}
