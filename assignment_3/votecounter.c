/*login: swann013, tangx254
* date: 04/11/18
* name: Kristopher Swann, Joseph Tang
* login: swann013, tangx254 */

#define _BSD_SOURCE
#define NUM_ARGS 3
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

int main(int argc, char **argv) {
  if (argc != NUM_ARGS + 1) {
    printf("Incorrect number of arguments supplied.\n");
    printf("Usage: ./votecounter <DAG.txt> <input_dir> <output_dir>\n");
    exit(1);
  }

  // Build graph from input file.
  struct dag_node_t* root = parse_dag_file(argv[1], INIT_NUM_CHILDREN);
}
