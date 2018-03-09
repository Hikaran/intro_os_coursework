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

#include "util.h"
#include "votes.h"

#define MAX_STRING_LEN 1024

/** 
 * Checks if a directory represents a leaf node. Such directories contain a
 * file called votes.txt and have no subdirectories.
 *
 * Takes a directory to be checked as an argument.
 *
 * Returns 0 for false and 1 for true.
 */ 
int is_leaf_node(DIR* dir) {
  struct dirent *entry;
  errno = 0;  // Reset errno so it can be used to check loop exit condition.
  int has_votes_file = 0;

  while (entry = readdir(dir)) {
    if (entry->d_type == DT_DIR) {
      // Skip parent and current directories.
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
        continue;
      } else {
        return 0;  // Subdirectory found; return false.
      }
    } else if (entry->d_type == DT_REG && strcmp(entry->d_name, "votes.txt") == 0) {
      has_votes_file = 1;
    }
  }

  // Handles errors with readdir().
  if (errno) {
    perror("Unexpected issue with readdir()");
    exit(1);
  }

  return has_votes_file;
}

/** 
 * Writes the results to a file.
 *
 * Takes a linked list of type struct votes as input.
 */ 
void record_votes(struct votes* results, char* path) {
  char record[MAX_STRING_LEN];

  while (results != NULL) {
    if (strlen(record) == 0) {
      sprintf(record, "%s:%d", results->candidate, results->votes);
    } else {
      sprintf(record + strlen(record), ",%s:%d", results->candidate, results->votes);
    }

    results = results->next;
  }

  char cur_dir[MAX_STRING_LEN];
  put_last_seperator(cur_dir, path, "/");
  char record_path[MAX_STRING_LEN];
  sprintf(record_path, "%s/%s.txt", path, cur_dir);

  FILE* recording = fopen(record_path, "w");
  if (recording == NULL) {
    perror("Could not create Leaf_Counter output file");
    printf("Encountered error in directory <%s>\n", path);
    exit(1);
  } 

  fprintf(recording, "%s\n", record);
  fclose(recording);
}

/** 
 * Counts the number of votes for each candidate and stores the results
 * in a linked list of type struct votes.
 */ 
void tally_votes(FILE* votes, char* path) {
  char candidate[MAX_STRING_LEN];
  struct votes* results = NULL;

  while (fgets(candidate, MAX_STRING_LEN, votes)) {
    // Skip empty lines.
    trimwhitespace(candidate);
    if (isspace(candidate[0])) {
        continue;
    }

    if (results == NULL) {
      results = add_votes(results, candidate, 1);
    } else {
      add_votes(results, candidate, 1);
    }
  }

  fclose(votes);
  record_votes(results, path);
  free_votes(results);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: ./Leaf_Counter <path>\n");
    exit(1);
  }

  // Make sure path resolves to a valid directory.
  char* path = argv[1];
  DIR* dir = opendir(path);
  if (!dir) {
    perror("Leaf_Counter failed to open directory");
    printf("Encountered error in directory <%s>\n", path);
    exit(1);
  }

  // Make sure directory represents a leaf node.
  if (!is_leaf_node(dir)) {
    printf("Not a leaf node.\n");
    exit(1);
  } 

  // Open the file containing the votes and pass it to the counting function.
  char vote_file_path[MAX_STRING_LEN];
  sprintf(vote_file_path, "%s/votes.txt", path);
  FILE* votes = fopen(vote_file_path, "r");
  if (votes == NULL) {
    perror("Could not open votes.txt");
    printf("Encountered error in path <%s>\n", path);
    exit(1);
  } 

  tally_votes(votes, path);

  char cur_dir[MAX_STRING_LEN];
  put_last_seperator(cur_dir, path, "/");
  printf("%s/%s.txt\n", path, cur_dir);

  return 0;
}
