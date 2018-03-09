/*login: swann013, tangx254
* date: 03/06/18
* name: Kristopher Swann, Joseph Tang
* login: swann013, tangx254 */

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

#include "votes.c"
#include "util.h"

#define MAX_STRING_LEN 1024

void append_winner(char* path) {
  // Get output file
  char cur_dir[MAX_STRING_LEN];
  put_last_seperator(cur_dir, path, "/");
  char output_path[MAX_STRING_LEN];
  sprintf(output_path, "%s/%s.txt", path, cur_dir);

  FILE *results_file = fopen(output_path, "a+e");
  if (results_file == NULL) {
    printf("Error opening file %s\n", output_path);
    exit(0);
  }

  // Get results string
  char results_str[MAX_STRING_LEN];
  fgets(results_str, MAX_STRING_LEN, results_file);

  // Find winner
  struct votes* results = NULL;
  results = add_votes_from_string(results_str, results);
  struct votes* winner = get_winner(results);
  if (winner == NULL) {
    printf("Could not parse any votes from file\n");
    fclose(results_file);
    exit(1);
  }

  // Append to file
  fprintf(results_file, "Winner:%s\n", winner->candidate);
  fclose(results_file);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: ./Vote_Counter <path>\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    perror("Error forking");
    exit(1);
  } else if (pid == 0) {
    // Child
    execl("./Aggregate_Votes", "Aggregate_Votes", argv[1], (char*) NULL);
    perror("Error after exec");
  }

  wait_for_all_children();
  append_winner(argv[1]);

  return 0;
}
