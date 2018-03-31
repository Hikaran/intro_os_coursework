#ifndef TEST_DECRYPT_H
#define TEST_DECRYPT_H

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "decrypt.h"

#define MAX_STRING_LEN 1024

void test_decrypt_example_from_writeup() {
  printf("test_example_from_writeup() ");

  char source[] = "YQCCJ";
  char target[MAX_STRING_LEN];
  char expected[] = "ASEEL";
  decrypt(source, target);
  assert(strcmp(target, expected) == 0 &&
      "test_example_from_writeup");

  printf("OK\n");
}

void test_decrypt_yz() {
  printf("test_decrypt_yz() ");

  char source[] = "ABYZ";
  char target[MAX_STRING_LEN];
  char expected[] = "CDAB";
  decrypt(source, target);
  assert(strcmp(target, expected) == 0 &&
      "test_example_from_writeup");

  printf("OK\n");
}

void test_decrypt_empty_string() {
  printf("test_decrypt_empty_string() ");

  char source[] = "";
  char target[MAX_STRING_LEN];
  char expected[] = "";
  decrypt(source, target);
  assert(strcmp(target, expected) == 0 &&
      "test_example_from_writeup");

  printf("OK\n");
}

/** Run the tests in this suite. */
void test_decrypt_runner() {
  test_decrypt_example_from_writeup();
  test_decrypt_yz();
  test_decrypt_empty_string();
}

#endif
