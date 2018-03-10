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
 * Tests if a symbolic link forms a cycle. If so, appends a message to
 * the output file.
 *
 * Exits upon completion because the process was created specifically to
 * check the link.
 *
 * The first argument is the path to the directory containing the link.
 * The second argument is the path to the Vote_Counter output file.
 * The third argument is the name of a link.
 */
void find_cycle(char* path, char* output_path, char* link_name) {
  // Save original working directory.
  char home_path[MAX_STRING_LEN];
  if (!getcwd(home_path, MAX_STRING_LEN)) {
    perror("Failed to get working directory");
    exit(0);  // Cycle check failure should not corrupt results.
  }

  // Change working directory to link location.
  if (chdir(path) != 0) {
    perror("Failed to enter link location");
    exit(0);  // Cycle check failure should not corrupt results.
  }

  // Retrieve absolute path to link location.
  char source[MAX_STRING_LEN];
  if (!getcwd(source, MAX_STRING_LEN)) {
    perror("Failed to get link location");
    exit(0);  // Cycle check failure should not corrupt results.
  }

  // Construct path to link.
  char link_path[MAX_STRING_LEN];
  sprintf(link_path, "%s/%s", source, link_name);

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

  // Break both paths into directory strings.
  char** link_source;
  int source_len = makeargv(source, "/", &link_source);
  char** link_destination;
  int destination_len = makeargv(destination, "/", &link_destination);
  int i = 0;

  // Compare each directory name iteratively.
  while (i < source_len && i < destination_len) {
    // Clean up and exit if there is a mismatch.
    if (strcmp(link_source[i], link_destination[i]) != 0) {
      freemakeargv(link_source);
      freemakeargv(link_destination);
      exit(0);
    } else {
      i++;
    }
  }

  // Cycle present since one path fully contains the other.
  // Restore original working directory.
  if (chdir(home_path) != 0) {
    perror("Failed to return to original working directory");
    exit(0);  // Cycle check failure should not corrupt results.
  }

  // Append cycle notification to file.
  FILE* recording = fopen(output_path, "a+");
  fprintf(recording,
          "There is a cycle from %s to %s.\n",
          link_source[source_len-1],
          link_destination[destination_len-1]);
  if (recording == NULL) {
    printf("Results file could not be opened in cycle check.");
    exit(0);
  }
  fclose(recording);
  freemakeargv(link_source);
  freemakeargv(link_destination);
  exit(0);
}

/**
 * Searches a directory tree for symbolic links.
 *
 * Assumes that all symbolic links lead to directories.
 *
 * The first argument is the root of the directory tree being searched.
 * The second argument is the path to the Vote_Counter output file.
 */
void find_links(char* path, char* output_path) {
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
      // Spawn a child process to check the link.
      pid = fork();
      if (pid < 0) {
        // Missing a link check does not invalidate results.
        // Print warning and continue execution.
        perror("Fork failed in link testing");
        continue;
      } else if (pid == 0) {
        find_cycle(path, output_path, entry->d_name);
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
  find_links(path, output_path);

  // Print output file name.
  printf("%s\n", output_path);

  return 0;
}
