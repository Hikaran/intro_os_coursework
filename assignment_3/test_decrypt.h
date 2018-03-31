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
  assert(strcmp(target, expected) == 0 && "test example from writeup");

  printf("OK\n");
}

void test_decrypt_yz() {
  printf("test_decrypt_yz() ");

  char source[] = "ABYZabyz";
  char target[MAX_STRING_LEN];
  char expected[] = "CDABcdab";
  decrypt(source, target);
  assert(strcmp(target, expected) == 0 &&
      "y and z should decrpyt to a and b");

  printf("OK\n");
}

void test_decrypt_empty_string() {
  printf("test_decrypt_empty_string() ");

  char source[] = "";
  char target[MAX_STRING_LEN];
  char expected[] = "";
  decrypt(source, target);
  assert(strcmp(target, expected) == 0 &&
      "empty string should decrypt to empty string");

  printf("OK\n");
}

void test_decrypt_numbers() {
  printf("test_decrypt_numbers() ");

  char source[] = "ABC1234DEF567";
  char target[MAX_STRING_LEN];
  char expected[] = "CDE1234FGH567";
  decrypt(source, target);
  assert(strcmp(target, expected) == 0 &&
      "numbers should not change during decryption");

  printf("OK\n");
}

void test_decrypt_whitespace() {
  printf("test_decrypt_numbers() ");

  char source[] = "A B C\nD\nE F";
  char target[MAX_STRING_LEN];
  char expected[] = "C D E\nF\nG H";
  decrypt(source, target);
  assert(strcmp(target, expected) == 0 &&
      "whitespace should not change during decryption");

  printf("OK\n");
}

void test_decrypt_special_chars() {
  printf("test_decrypt_numbers() ");

  char source[] = "AB!@#$%^&(*()(|YZ";
  char target[MAX_STRING_LEN];
  char expected[] = "CD!@#$%^&(*()(|AB";
  decrypt(source, target);
  assert(strcmp(target, expected) == 0 &&
      "special characters should not change during decryption");

  printf("OK\n");
}

/** Run the tests in this suite. */
void test_decrypt_runner() {
  test_decrypt_example_from_writeup();
  test_decrypt_yz();
  test_decrypt_empty_string();
  test_decrypt_numbers();
  test_decrypt_whitespace();
  test_decrypt_special_chars();
}

#endif
