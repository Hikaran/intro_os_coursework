#ifndef DECRYPT_H
#define DECRYPT_H

#include <string.h>
#include <ctype.h>

#define OFFSET 2
#define WRAPLENGTH 26

void decrypt(const char *source, char* target) {
  // If char is alpha, decrypt by OFFSET by copying over
  for (int i = 0; i < strlen(source); i++) {
    char result = source[i];
    if (isalpha(result)) {
      result += OFFSET;
      // If no longer alpha, moved past end of alphabet, wrap back
      while (!isalpha(result)) {
        result -= WRAPLENGTH;
      }
    }
    target[i] = result;
  }

  // Make sure to add string terminator on end of target
  target[strlen(source)] = '\0';
}

#endif
