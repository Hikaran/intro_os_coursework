/*login: swann013, tangx254
* date: 03/06/18
* name: Kristopher Swann, Joseph Tang
* login: swann013, tangx254 */

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#include "votes.h"
#include "util.h"

#define MAX_STRING_LEN 1024

void silence_output() {
  int fd = open("/dev/null", O_WRONLY);
  if (dup2(fd, 1) < 0) {
    // error handling
    perror("Could not redirect stdout");
    exit(1);
  }
  if (dup2(fd, 2) < 0) {
    // error handling
    perror("Could not redirect stderr");
    exit(1);
  }
}

/**
 * Return if dir is a leaf node.
 * A dir is a leaf node if it has no subdirs and contains a votes.txt file.
 */
int is_leaf_node(DIR* dir) {
  rewinddir(dir);
  struct dirent *entry;
  errno = 0;  // Reset errno
  int has_votes_file = 0;

  while (entry = readdir(dir)) {
    if (entry->d_type == DT_DIR) {
      // Ignore "." and ".." dirs
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
        continue;
      }
      return 0;  // Return false, contains subdirs
    }
    if (entry->d_type == DT_REG && strcmp("votes.txt", entry->d_name) == 0) {
      has_votes_file = 1;
    }
  }
  if (errno) {
    perror("readdir() failed");
    exit(1);
  }
  rewinddir(dir);

  if (has_votes_file) {
    return 1;
  }
  return 0;  // Return false, missing votes.txt
}

/**
 * Run Aggregate_Votes on all subdirs.
 *
 * Each subdir will create a new process and rerun Aggregate_Votes.
 */
void aggregate_sub_dirs(char* path, DIR* dir) {
  rewinddir(dir);
  struct dirent *entry;
  errno = 0;  // Reset errno

  // For each subdir fork, parent continues, child execs on subdir
  while (entry = readdir(dir)) {
    // Ignore "." and ".." dirs and non dir entries
    if (strcmp(entry->d_name, ".") == 0 ||
        strcmp(entry->d_name, "..") == 0 ||
        entry->d_type != DT_DIR) {
      continue;
    }

    int pid = fork();
    if (pid < 0) {
      perror("Error forking");
      exit(1);
    } else if (pid == 0) {
      // Child
      char newpath[MAX_STRING_LEN];
      sprintf(newpath, "%s/%s", path, entry->d_name);
      silence_output();
      execl("./Aggregate_Votes", "Aggregate_Votes", newpath, (char*) NULL);
      perror("Error after exec");
      exit(1);
    }
  }
  if (errno) {
    perror("readdir() failed");
    exit(1);
  }
  rewinddir(dir);

  wait_for_all_children();
}

/** Read first line of file at path into buf. */
void read_first_line(char* buf, char* path) {
  FILE *results = fopen(path, "re");
  if (results == NULL) {
    printf("Error opening file %s\n", path);
    exit(1);
  }
  fgets(buf, MAX_STRING_LEN, results);
  fclose(results);
}

/** Write results to current dir. */
void write_results_to_dir(char* path, struct votes *head) {
  // Get output file
  char cur_dir[MAX_STRING_LEN];
  put_last_seperator(cur_dir, path, "/");
  char sum_path[MAX_STRING_LEN];
  sprintf(sum_path, "%s/%s.txt", path, cur_dir);
  printf("%s\n", sum_path);

  FILE *sum_results = fopen(sum_path, "we");
  if (sum_results == NULL) {
    printf("Error opening file %s\n", sum_path);
    exit(1);
  }

  // Write results to file
  char sum_str[MAX_STRING_LEN] = {'\0'};
  to_string(sum_str, head);
  fprintf(sum_results, "%s\n", sum_str);
  fclose(sum_results);
}

void aggregate_cur_dir(char* path, DIR* dir) {
  rewinddir(dir);
  struct dirent *entry;
  struct votes* sum_votes = NULL;
  errno = 0;  // Reset errno

  // Sum results in each subdir
  while (entry = readdir(dir)) {
    // Ignore "." and ".." dirs and non dir entries
    if (strcmp(entry->d_name, ".") == 0 ||
        strcmp(entry->d_name, "..") == 0 ||
        entry->d_type != DT_DIR) {
      continue;
    }

    // Get path to results in subdir
    char subdir_results_path[MAX_STRING_LEN];
    sprintf(subdir_results_path, "%s/%s/%s.txt", path, entry->d_name, entry->d_name);

    // Add results and update head if necessary
    char buf[MAX_STRING_LEN];
    read_first_line(buf, subdir_results_path);
    if (sum_votes == NULL) {
      sum_votes = add_votes_from_string(buf, sum_votes);
    } else {
      add_votes_from_string(buf, sum_votes);
    }
  }
  if (errno) {
    perror("Could not sum results");
    exit(1);
  }

  write_results_to_dir(path, sum_votes);
  rewinddir(dir);
}

void run_leaf_node(char* path) {
      silence_output();
      execl("./Leaf_Counter", "Leaf_Counter", path, (char*) NULL);
      perror("Leaf_Counter exec failure");
      exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: ./Aggregate_Votes <path>\n");
    exit(1);
  }
  char* path = argv[1];

  // Make sure path is a valid dir
  DIR* dir = opendir(path);
  if (!dir) {
    perror("Failed to open initial directory");
    exit(1);
  }

  if (is_leaf_node(dir)) {
    run_leaf_node(path);
  } else {
    aggregate_sub_dirs(path, dir);
    aggregate_cur_dir(path, dir);
  }

  return 0;
}
