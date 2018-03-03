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

#include "util.h"
#include "votes.c"

#define MAX_STRING_LEN 1024

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

/** Wait for all child proccess to finish running. */
void wait_for_all_children() {
  while (1) {
    errno = 0;  // Reset errno
    int status;
    int wait_pid = wait(&status);
    if (wait_pid == -1) {
      // No more children can exit loop
      if (errno == ECHILD) {
        break;
      }
    } else {
      if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        perror("Child process exited abnormally");
        exit(1);
      }
    }
  }
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
      execl("./Aggregate_Votes", "Aggregate_Votes", newpath, (char*) NULL);
      perror("Error after exec");
    }
  }
  if (errno) {
    perror("readdir() failed");
    exit(1);
  }
  rewinddir(dir);

  wait_for_all_children();
}

void aggregate_cur_dir(char* path, DIR* dir) {
  printf("agg node: %s\n", path);
}

void run_leaf_node(char* path) {
  printf("leaf node: %s\n", path);
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

  /* if (is_leaf_node(dir)) { */
  /*   run_leaf_node(path); */
  /* } else { */
  /*   aggregate_sub_dirs(path, dir); */
  /*   aggregate_cur_dir(path, dir); */
  /* } */


  struct votes* head = NULL;

  head = add_votes(head, "abc", 3);
  add_votes(head, "abc", 4);
  add_votes(head, "def", 5);

  printf("head=<%s,%d>\n", head->candidate, head->votes);
  printf("head->next=<%s,%d>\n", head->next->candidate, head->next->votes);

  char buf[1024] = {'\0'};
  to_string(buf, head);
  printf("to_string=%s\n", buf);
  free_votes(head);

  return 0;
}
