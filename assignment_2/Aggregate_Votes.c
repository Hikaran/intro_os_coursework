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

/**
 * Return if dir is a leaf node.
 * A dir is a leaf node if it has no subdirs and contains a votes.txt file.
 * Dir is reset (rewinddir) after call.
 */
int is_leaf_node(DIR* dir) {
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

  if (has_votes_file) {
    return 1;
  }
  return 0;  // Return false, missing votes.txt
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: ./Aggregate_Votes <path>\n");
    exit(1);
  }

  // Make sure path resolves to a valid directory. If not, throw an error and exit.
  DIR* dir = opendir(argv[1]);
  if (!dir) {
    perror("Failed to open initial directory");
    exit(1);
  }

  if (is_leaf_node(dir)) {
    printf("is leaf node\n");
  } else {
    printf("not leaf node\n");
  }


  return 0;
}
