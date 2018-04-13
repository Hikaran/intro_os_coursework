#ifndef TALLY_H
#define TALLY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 2048

struct tally {
  char name[MAX_STR_LEN];
  int count;
  struct tally* next;
};

/**
 * Increase tally for the given alias in the given linked list.
 */
struct tally* add_items(struct tally* record, char* name, int count) {
  // Base case for empty list.
  if (record == NULL) {
    record = malloc(sizeof(struct tally));
    strcpy(record->name, name);
    record->count = count;
    record->next = NULL;
    return record;
  }

  struct tally* entry = record;

  // Match found, update current node
  if (strcmp(record->name, name) == 0) {
    record->count += count;
    return record;
  }
  // No match found and end of list, create new node at end
  if (record->next == NULL) {
    record->next = add_items(record->next, name, count);
    return record->next;
  }
  // No match found and not end of list, keep searching.
  return add_items(record->next, name, count);
}

/**
 * Iteratively free linked list of tallies.
 */
void free_tally(struct tally* record) {
  while (record != NULL) {
    struct tally* next = record->next;
    free(record);
    record = NULL;
    record = next;
  }
}

#endif
