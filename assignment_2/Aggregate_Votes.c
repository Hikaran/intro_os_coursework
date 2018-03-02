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
    printf("Usage: ./Aggregate_Votes <path>\n");
    exit(1);
  }

  DIR* dir = opendir(argv[1]);
  if (!dir) {
    perror("Failed to open initial directory");
    exit(1);
  } else {
    printf("Success!\n");
  }


  return 0;
}
