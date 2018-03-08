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

#include "util.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: ./Leaf_Counter <path>\n");
    exit(1);
  }

  // Make sure path resolves to a valid directory. If not, throw an error and exit.
  DIR* dir = opendir(argv[1]);
  if (!dir) {
    perror("Failed to open initial directory");
    exit(1);
  }

  struct dirent *entry;

  return 0;
}
