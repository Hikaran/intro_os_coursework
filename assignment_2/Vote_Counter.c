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

void append_winner(char* path, char* output_path) {
  FILE *results_file = fopen(output_path, "a+e");
  if (results_file == NULL) {
    printf("Error opening file %s\n", output_path);
    exit(1);
  }

  // Get results string
  char results_str[MAX_STRING_LEN];
  fgets(results_str, MAX_STRING_LEN, results_file);

  // Find winner
  struct votes* results = NULL;
  results = add_votes_from_string(results_str, results);
  struct votes* winner = get_winner(results);
  free_votes(results);
  if (winner == NULL) {
    printf("Could not parse any votes from file\n");
    fclose(results_file);
    exit(1);
  }

  // Append to file
  fprintf(results_file, "Winner:%s\n", winner->candidate);
  fclose(results_file);
}

/**
 * Searches a directory tree for symbolic links that create a cycle.
 *
 * Assumes that all symbolic links lead to directories.
 *
 * First argument is the root of the directory tree being searched.
 * Second argument is the path to the output file.
 */
void find_cycles(char* path, char* output_path) {
  // Make sure path leads to a valid directory.
  DIR* dir = opendir(path);
  if (!dir) {
    // Missing potential cycles does not invalidate results.
    // Print warning and exit function.
    perror("Failed to open initial directory in cycle check");
    return;
  }

  struct dirent *entry;
  int depth = 0;
  int pid = 0;

  while ((entry = readdir(dir))) {
    if (entry->d_type == DT_DIR) {
      // Ignore "." and ".." dirs
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
        continue;
      } else {
        // TODO Delete this print statement; proof-of-concept only.
        printf("Cycle check found dir |%s| in |%s|\n", entry->d_name, path);
        // Spawn a child process to search subdirectory.
        pid = fork();
        if (pid < 0) {
          // Missing a subdirectory here does not invalidate results.
          // Print warning and continue execution.
          perror("Fork failed in cycle detection");
          continue;
        } else if (pid == 0) {  // Child branch.
          depth++;
          char new_path[MAX_STRING_LEN];
          sprintf(new_path, "%s/%s", path, entry->d_name);
          path = new_path;
          // Reopen directory stream using new path.
          closedir(dir);
          dir = opendir(path);
          // Fatal error for child process.
          if (!dir) {
            perror("Failed to open subdirectory in cycle check");
            exit(0);  // Parent should continue searching for cycles.
          }
        }
      }
    } else if (entry->d_type == DT_LNK) {
      // TODO Delete this print statement; proof-of-concept only.
      printf("Cycle check found link |%s| in |%s|\n", entry->d_name, path);

      // Spawn a child process to check the link.
      pid = fork();
      if (pid < 0) {
        // Missing a link check does not invalidate results.
        // Print warning and continue execution.
        perror("Fork failed in link testing");
        continue;
      } else if (pid == 0) {
        // Change working directory to link location.
        if (chdir(path) != 0) {
          perror("Failed to enter link location");
          exit(0);  // Cycle check failure should not corrupt results.
        }

        // Retrieve absolute path to link location.
        char source[MAX_STRING_LEN];
        if (!getcwd(source, MAX_STRING_LEN)) {
          perror("Failed to get working directory");
          exit(0);  // Cycle check failure should not corrupt results.
        }

        // Construct path to link.
        char link_path[MAX_STRING_LEN];
        sprintf(link_path, "%s/%s", source, entry->d_name);

        // Change working directory to link destination.
        // chdir resolves symbolic links.
        if (chdir(link_path) != 0) {
          perror("Failed to follow link");
          exit(0);  // Cycle check failure should not corrupt results.
        }

        // Retrieve absolute path to link destination.
        char destination[MAX_STRING_LEN];
        if (!getcwd(destination, MAX_STRING_LEN)) {
          perror("Failed to get link destination");
          exit(0);  // Cycle check failure should not corrupt results.
        }

        // TODO Delete print statements.
        printf("Source directory |%s|\n", source);
        printf("Link directory |%s|\n", destination);

        // Check if link destination is a parent of link source.
        

        exit(0);
      }
    }
  }

  wait_for_all_children();

  // Only the original process should continue.
  if (depth != 0) {
    exit(0);
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: ./Vote_Counter <path>\n");
    exit(1);
  }

  char* path = argv[1];

  int pid = fork();
  if (pid < 0) {
    perror("Error forking");
    exit(1);
  } else if (pid == 0) {
    // Child
    silence_output();
    execl("./Aggregate_Votes", "Aggregate_Votes", path, (char*) NULL);
    perror("Exec failure in Vote_Counter");
    exit(1);
  }

  wait_for_all_children();

  // Generate output file path.
  char cur_dir[MAX_STRING_LEN];
  put_last_seperator(cur_dir, path, "/");
  char output_path[MAX_STRING_LEN];
  sprintf(output_path, "%s/%s.txt", path, cur_dir);

  append_winner(path, output_path);
  find_cycles(path, output_path);

  // Print output file name.
  printf("%s\n", output_path);

  return 0;
}
