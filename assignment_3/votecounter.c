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
#include "rmrf.h"
#include "queue.h"
#include "decrypt.h"
#include "tally.h"

struct thread_args {
  char* input_dir_name;
  char* output_dir_name;
  struct queue_t* queue;
  struct dag_node_t* root;
};

/**
 * Add files from directory to queue.
 *
 * Returns the number of files placed in queue.
 */
int queue_files(struct queue_t* queue, struct dag_node_t* root, char* dir_name) {
  DIR* dir = opendir(dir_name);
  if (dir == NULL) {
    perror("Could not open input directory");
    exit(1);
  }

  struct dirent* entry;
  char file_path[MAX_PATH];
  int num_files = 0;
  errno = 0;  // Reset errno

  while (entry = readdir(dir)) {
    // Ignore entries that do not correspond to regular files.
    if (entry->d_type != DT_REG) {
      continue;
    }

    // Check if file corresponds to leaf node.
    struct dag_node_t* node = find_node(root, entry->d_name);
    if (node == NULL || node->num_children != 0) {
      continue;
    }

    // Check if file contains votes.
    sprintf(file_path, "%s/%s", dir_name, entry->d_name);
    FILE* input_file = fopen(file_path, "r");
    if (input_file == NULL) {
      char error_msg[MAX_PATH];
      sprintf(error_msg, "Could not verify if %s contains votes", entry->d_name);
      perror(error_msg);
      exit(1);
    }

    char line[MAX_STR_LEN];
    while(fgets(line, MAX_STR_LEN, input_file)) {
      // Skip empty lines.
      trimwhitespace(line);
      if (isspace(line[0])) {
          continue;
      }
      // At least one vote found. Add file to queue.
      enqueue(queue, entry->d_name);
      num_files++;
      break;
    }
    if (fclose(input_file)) {
      perror("Failed to close a file after queueing");
    }
  }

  if (errno) {
    perror("Unexpected issue with readdir()");
    exit(1);
  }

  return num_files;
}

/*
 * Call decrypt helper function line by line to translate file.
 */
void decrypt_file(FILE* source, FILE* target) {
  char source_line[MAX_STR_LEN];
  char target_line[MAX_STR_LEN];
  while(fgets(source_line, MAX_STR_LEN, source)) {
    trimwhitespace(source_line);
    if (isspace(source_line[0])) {
      continue;
    }
    decrypt(source_line, target_line);
    fprintf(target, "%s\n", target_line);
  }
}

void record_votes(char* path, struct tally* record) {
  
}

/**
 * Main function for child thread execution.
 *
 * 1) Retrieve file name from queue.
 * 2) Decrypt input file.
 * 3) Place decrypted file in output directory.
 * 4) Aggregate results iteratively until root node.
 */
void* run_child_thread(void* args) {
  struct thread_args* input = (struct thread_args*) args;

  // Retrieve file name.
  char* file_name = (char*)malloc(MAX_PATH*sizeof(char));
  dequeue(input->queue, file_name);

  // Log starting message. TODO
  char log_message[MAX_PATH];
  pthread_t tid = pthread_self();
  sprintf(log_message, "%s:%lu:start", file_name, tid);
  printf("%s\n", log_message);

  // Construct input file path.
  char input_path[MAX_PATH];
  sprintf(input_path, "%s/%s", input->input_dir_name, file_name);

  // Construct output file path.
  char output_path[MAX_PATH];
  struct dag_node_t* node = find_node(input->root, file_name);
  sprintf(output_path, "%s/%s/%s.txt", input->output_dir_name, node->path, file_name);

  pthread_mutex_lock(&node->mutex);
  FILE* input_file = fopen(input_path, "r");
  if (input_file == NULL) {
    char error_msg[MAX_STR_LEN];
    sprintf(error_msg, "Failed to open input %s for decryption", file_name);
    perror(error_msg);
    exit(1);
  }

  FILE* output_file = fopen(output_path, "w");
  if (output_file == NULL) {
    char error_msg[MAX_STR_LEN];
    sprintf(error_msg, "Failed to open output %s for decryption", file_name);
    perror(error_msg);
    exit(1);
  }

  // Run decryption algorithm.
  decrypt_file(input_file, output_file);
  if (fclose(input_file)) {
    perror("Failed to close input file after decryption");
  }
  if (fclose(output_file)) {
    perror("Failed to close output file after decryption");
  }

  // Tally votes in leaf node.
  output_file = fopen(output_path, "r");
  if (output_file == NULL) {
    char error_msg[MAX_STR_LEN];
    sprintf(error_msg, "Failed to open output %s for tallying", file_name);
    perror(error_msg);
    exit(1);
  }

  char line[MAX_STRING_LEN];
  struct tally* votes = NULL;
  while(fgets(line, MAX_STRING_LEN, output_file)) {
    // Skip empty lines.
    trimwhitespace(line);
    if (isspace(line[0])) {
        continue;
    }
    if (votes == NULL) {
      votes = add_items(votes, line, 1);
    } else {
      add_items(votes, line, 1);
    }
  }

  struct tally* viewer;
  viewer = votes;
  while (viewer != NULL) {
    printf("%s has %d votes.\n", viewer->name, viewer->count);
    viewer = viewer->next;
  }

  if (fclose(output_file)) {
    perror("Failed to close output file after tallying");
  }

  pthread_mutex_unlock(&node->mutex);

  // Aggregate results upward.
  while (node != input->root) {
    node = node->parent;
    pthread_mutex_lock(&node->mutex);

    // Tally votes in intermediate node.
    sprintf(output_path, "%s/%s/%s.txt", input->output_dir_name, node->path, node->name);
    record_votes(output_path, votes);

    pthread_mutex_unlock(&node->mutex);
  }

  // Log finishing message. TODO
  sprintf(log_message, "%s:%lu:end", file_name, tid);
  printf("%s\n", log_message);

  free_tally(votes);
  free(file_name);
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
  int num_threads = queue_files(file_queue, root, input_dir_name);

  // Throw error if no valid leaf files found.
  if (num_threads < 1) {
    printf("error:input directory is empty\n");
    exit(1);
  }

  // Initialize threads.
  pthread_t threads[num_threads];
  struct thread_args child_args[num_threads];

  for (int i = 0; i < num_threads; i++) {
    child_args[i].input_dir_name = input_dir_name;
    child_args[i].output_dir_name = output_dir_name;
    child_args[i].queue = file_queue;
    child_args[i].root = root;
    pthread_create(&threads[i], NULL, run_child_thread, &child_args[i]);
  }

	// Make main thread wait for each spawned thread.
	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}

  free_queue(file_queue);
  free(file_queue);
  free_dag(root);
}
