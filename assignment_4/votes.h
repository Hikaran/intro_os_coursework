#ifndef VOTES_H
#define VOTES_H

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

#include "makeargv.h"
#include "util.h"

#define MAX_STRING_LEN 1024

/** Singly linked list for counting votes. */
typedef struct votes {
  char candidate[MAX_STRING_LEN];
  int votes;
  struct votes* next;
} vote_t;

/**
 * Add votes for a candidate.
 * Creates new nodes if needed.
 * Returns pointer to updated/created node.
 * Recursively traverses linked list.
 */
struct votes* add_votes(struct votes *head, char *candidate, int num_votes) {
  // Base case
  if (head == NULL) {
    head = (struct votes*)malloc(sizeof(struct votes));
    strcpy(head->candidate, candidate);
    head->votes = num_votes;
    head->next = NULL;
    return head;
  }
  // Match found, update current node
  if (strcmp(head->candidate, candidate) == 0) {
    head->votes += num_votes;
    return head;
  }
  // No match found and end of list, create new node at end
  if (head->next == NULL) {
    head->next = add_votes(head->next, candidate, num_votes);
    return head->next;
  }
  // No match found and not end of list, keep searching.
  return add_votes(head->next, candidate, num_votes);
}

/** Free all nodes in the linked list */
void free_votes(struct votes *head) {
  // Base case
  if (head == NULL) {
    return;
  }
  free_votes(head->next);
  free(head);
}

/**
 * Fill buf with string representation of votes.
 * "A:2,B:3,C:5"
 * Make sure to initalize buf as empty string (char buf[50] = {'\0'};)
 */
void votes_to_string(char *buf, struct votes *head) {
  // Base case
  if (head == NULL) {
    return;
  }
  // If first, no leading comma
  if (strlen(buf) == 0) {
    sprintf(buf, "%s:%d", head->candidate, head->votes);
  } else {
    sprintf(buf + strlen(buf), ",%s:%d", head->candidate, head->votes);
  }
  votes_to_string(buf, head->next);
}

/**
 * Add votes from string of form "A:7,B:2,C:3"
 * Return pointer to new head (if changed).
 */
struct votes* votes_add_from_string(char *str, struct votes *head) {
  // Split string into comma seperated segments
  char **node_strings;
  int num_nodes = makeargv(trimwhitespace(str), ",", &node_strings);

  for (int i = 0; i < num_nodes; i++) {
    // Split each comma seperated segment ("A:7") into parts
    /* char *cur_node_string = (*node_strings)[i]; */
    char *cur_node_string = trimwhitespace(node_strings[i]);
    char **node_split;
    int num_items = makeargv(cur_node_string, ":", &node_split);

    // Must have exactly 2 parts
    if (num_items != 2) {
      fprintf(stderr, "Formatting Error!\n");
      exit(1);
    }

    char *candidate = node_split[0];
    int num_votes = atoi(node_split[1]);

    // Add votes and update head if needed.
    if (head == NULL) {
      head = add_votes(head, candidate, num_votes);
    } else {
      add_votes(head, candidate, num_votes);
    }

    freemakeargv(node_split);
  }
  freemakeargv(node_strings);
  return head;
}

/** Read the votes file into a votes struct */
struct votes* votes_from_file(char* filepath) {
  FILE* file = fopen(filepath, "r");
  struct votes* results = NULL;
  if (file == NULL) {
    perror("Could not open file");
    exit(1);
  }

  char line[MAX_STRING_LEN];
  while (fgets(line, MAX_STRING_LEN, file)) {
    // Skip empty lines.
    char* candidate = trimwhitespace(line);
    if (isspace(line[0])) {
        continue;
    }

    if (results == NULL) {
      results = add_votes(results, candidate, 1);
    } else {
      add_votes(results, candidate, 1);
    }
  }

  fclose(file);
  return results;
}

struct votes* votes_get_winner(struct votes *head) {
  struct votes* leader = head;
  while (head != NULL) {
    if (head->votes > leader->votes) {
      leader = head;
    }
    head = head->next;
  }
  return leader;
}

#endif
