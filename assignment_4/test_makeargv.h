#ifndef TEST_MAKEARGV_H
#define TEST_MAKEARGV_H

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "makeargv.h"

#define MAX_STRING_LEN 1024

void test_makeargv_colons() {
  printf("test_makeargv_colons() ");

  char line[] = "this:is:a:colon:delimited:line";
  char** tokens;
  int num_tokens = makeargv(line, ":", &tokens);

  assert(num_tokens == 6 &&
      "should have split the line into 6 pieces");

  assert(strcmp(tokens[0], "this") == 0);
  assert(strcmp(tokens[1], "is") == 0);
  assert(strcmp(tokens[2], "a") == 0);
  assert(strcmp(tokens[3], "colon") == 0);
  assert(strcmp(tokens[4], "delimited") == 0);
  assert(strcmp(tokens[5], "line") == 0);

  freemakeargv(tokens);

  printf("OK\n");
}

void test_makeargv_colons_and_spaces() {
  printf("test_makeargv_colons_and_spaces() ");

  char line[] = "colon:and space delimited:line";
  char** tokens;
  int num_tokens = makeargv(line, " :", &tokens);

  assert(num_tokens == 5 &&
      "should have split the line into 6 pieces");

  assert(strcmp(tokens[0], "colon") == 0);
  assert(strcmp(tokens[1], "and") == 0);
  assert(strcmp(tokens[2], "space") == 0);
  assert(strcmp(tokens[3], "delimited") == 0);
  assert(strcmp(tokens[4], "line") == 0);

  freemakeargv(tokens);

  printf("OK\n");
}

/** Run the tests in this suite. */
void test_makeargv_runner() {
  test_makeargv_colons();
  test_makeargv_colons_and_spaces();
}

#endif
