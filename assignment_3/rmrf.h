#ifndef RMRF_H
#define RMRF_H

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>

/**
 * An implementation of rm -rf using nftw
 */

static int rm_helper(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb) {
  if(remove(pathname) < 0) {
    perror("ERROR: remove for rmrf");
    exit(1);
  }
  return 0;
}

void rmrf(const char* pathname) {
  // Traverse file tree in reverse order and remove entries
  if (nftw(pathname, rm_helper, 100, FTW_DEPTH|FTW_MOUNT|FTW_PHYS) < 0) {
    perror("ERROR: ntfw in rmrf");
    exit(1);
  }
}

#endif
