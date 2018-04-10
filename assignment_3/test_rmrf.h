#ifndef TEST_RMRF_H
#define TEST_RMRF_H

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include "rmrf.h"

void test_rmrf() {
  printf("test_rmrf() ");

  char* base_dir = "foobar";
  char* child_dir = "foobar/child";

  mkdir(base_dir, 0777);
  mkdir(child_dir, 0777);

  char* file1 = "foobar/file1.txt";
  char* file2 = "foobar/child/file2.txt";
  char* file3 = "foobar/child/file3.txt";

  FILE* file = NULL;

  file = fopen(file1, "w");
  fclose(file);
  file = fopen(file2, "w");
  fclose(file);
  file = fopen(file3, "w");
  fclose(file);

  DIR* dir = NULL;

  // Check directiors were created
  dir = opendir(base_dir);
  assert(dir != NULL &&
      "root folder should exist");
  closedir(dir);
  dir = opendir(child_dir);
  assert(dir != NULL &&
      "child folder should exist");
  closedir(dir);

  // Check that files were created
  file = fopen(file1, "r");
  assert (file != NULL &&
      "file1 should exist");
  fclose(file);
  file = fopen(file2, "r");
  assert (file != NULL &&
      "file2 should exist");
  fclose(file);
  file = fopen(file3, "r");
  assert (file != NULL &&
      "file3 should exist");
  fclose(file);

  rmrf(base_dir);

  // check directories were deleted
  dir = opendir(base_dir);
  assert(dir == NULL &&
      "root folder should no longer exist");
  dir = opendir(child_dir);
  assert(dir == NULL &&
      "child folder should no longer exist");

  // Check that files were deleted
  file = fopen(file1, "r");
  assert (file == NULL &&
      "file1 should exist");
  file = fopen(file2, "r");
  assert (file == NULL &&
      "file2 should exist");
  file = fopen(file3, "r");
  assert (file == NULL &&
      "file3 should exist");

  printf("OK\n");
}

/** Run the tests in this suite. */
void test_rmrf_runner() {
  test_rmrf();
}

#endif
