#ifndef DAG_H
#define DAG_H

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "rmrf.h"
#include "makeargv.h"
#include "util.h"

#define MAX_STR_LEN 1024
#define MAX_PATH 4096

struct dag_node_t {
  char* name;
  char* path;
  struct dag_node_t* parent;
  struct dag_node_t** children;
  int num_children;
  int max_children;
  pthread_mutex_t mutex;
};

/** 
 * Initialize a node in the graph via the following tasks.
 * 1) Allocate memory for node struct.
 * 2) Allocate memory for and save directory name.
 * 3) Allocate memory for path name and save directory name to path.
 * 4) Allocate memory for pointers to child nodes.
 * 5) Initialize mutex lock.
 *
 * Returns a reference to the created node.
 */
struct dag_node_t* init_dag_node(char* name, int max_children) {
  struct dag_node_t* node = (struct dag_node_t*)malloc(sizeof(struct dag_node_t));

  // Save directory name.
  node->name = (char*)malloc(strlen(name));
  if (name != NULL) {
    strcpy(node->name, name);
  } else {
    node->name = NULL;
  }

  // Add directory name to path.
  node->path = (char*)malloc(MAX_PATH);
    if (name != NULL) {
    strcpy(node->path, name);
  } else {
    node->path = NULL;
  }

  // Initialize array of child references.
  node->max_children = max_children;
  node->num_children = 0;
  node->children = (struct dag_node_t**)malloc(
      max_children * sizeof(struct dag_node_t*));

  // Initialize mutex
  if (pthread_mutex_init(&(node->mutex), NULL)) {
    perror("Could not initialize mutex");
    exit(1);
  }
  return node;
}

/**
 * Deallocate entire graph recursively.
 */
void free_dag(struct dag_node_t* root) {
  // Deallocate name field.
  if (root->name != NULL) {
    free(root->name);
    root->name == NULL;
  }

  // Deallocate path field.
  if (root->path != NULL) {
    free(root->path);
    root->path == NULL;
  }

  // Recursively deallocate each child node.
  for (int i = 0; i < root->num_children; i++) {
    free_dag(root->children[i]);
  }

  // Deallocate list of child references.
  free(root->children);
  root->children = NULL;

  // Deallocate data struct.
  free(root);
}

/**
 * Attach the given child node to the given parent node.
 */
void add_child_node(struct dag_node_t* child, struct dag_node_t* parent) {
  // Save parent reference to child.
  child->parent = parent;

  // If parent ran out of space, double the space and copy over contents.
  if (parent->num_children >= parent->max_children) {
    int new_max = 2 * parent->max_children;
    struct dag_node_t** new_arr = (struct dag_node_t**)malloc(
      new_max * sizeof(struct dag_node_t*));
    memcpy(new_arr, parent->children,
        parent->max_children * sizeof(struct dag_node_t*));
    free(parent->children);
    parent->children = new_arr;
    parent->max_children = new_max;
  }

  // Add child reference to parent.
  parent->children[parent->num_children] = child;
  parent->num_children++;

  // Add parent path to child path.
  char new_path[MAX_PATH];
  sprintf(new_path, "%s/%s", parent->path, child->path);
  strcpy(child->path, new_path);
}

/**
 * Recursively search for a node by name.
 *
 * If found, returns the matching node. Otherwise, returns NULL. 
 */
struct dag_node_t* find_node(struct dag_node_t* root, char* query_name) {
  if (root == NULL) {
    return NULL;
  }

  if (strcmp(root->name, query_name) == 0) {
    return root;
  }

  struct dag_node_t* found_node = NULL;
  for(int i = 0; i < root->num_children; i++) {
    struct dag_node_t* subquery = find_node(root->children[i], query_name);
    if (subquery != NULL) {
      found_node = subquery;
      break;
    }
  }
  return found_node;
}

/**
 * Link graph as specified by input line.
 *
 * Line format should be 'parent:child1:child2:child3...'
 *
 * Assumes that all parent nodes besides the root have appeared as children
 * in previously parsed lines.
 */
void parse_dag_line(struct dag_node_t* root, char* line, int max_children) {
  char** line_names;
  int num_args = makeargv(line, ":", &line_names);

  if (num_args < 2) {
    printf("Cannot parse line for dag config: '%s'\n", line);
    exit(1);
  }

  // First listed name is the parent node
  struct dag_node_t* parent_node = find_node(root, line_names[0]);
  if (NULL == parent_node) {
    printf("Could not find parent node '%s' in graph.\n", line_names[0]);
    exit(1);
  }

  // Add a NEW child node for each child node specified
  for (int i = 1; i < num_args; i++) {  // Start from 1 intentional
    char* child_name = line_names[i];
    trimwhitespace(child_name);
    struct dag_node_t* new_child = init_dag_node(child_name, max_children);
    add_child_node(new_child, parent_node);
  }

  freemakeargv(line_names);
}

/**
 * Generate graph from file.
 *
 * Assume that no non-root node appears as a parent before it appears as
 * a child. 
 */
struct dag_node_t* parse_dag_file(char* filename, int max_children) {
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    perror("Could not open dag file");
    exit(1);
  }

  char line[MAX_STR_LEN];

  // TODO Error handling
  // Two main cases I can think of here:
  // 1) First line is empty.
  // 2) All lines are empty.

  // Use the first token of the first line to set the root node
  if (!fgets(line, MAX_STR_LEN, file)) {
    perror("Could not read first line in dag file");
    exit(1);
  }
  char** first_line_parts;
  makeargv(line, ":", &first_line_parts);
  char* root_name = first_line_parts[0];
  trimwhitespace(root_name);
  struct dag_node_t* root = init_dag_node(root_name, max_children);
  freemakeargv(first_line_parts);

  rewind(file);  // Re-parse the first line to set up relationships

  // Parse the rest of the file
  while(fgets(line, MAX_STR_LEN, file)) {
    // Skip empty lines.
    trimwhitespace(line);
    if (isspace(line[0])) {
        continue;
    }
    parse_dag_line(root, line, max_children);
  }

  return root;
}

/**
 * Create the directory structure with the specified root node at the
 * specified path location.
 */
void create_dir_structure(struct dag_node_t* root, char* base_dir) {
  char dirname[MAX_STR_LEN];
  sprintf(dirname, "%s/%s", base_dir, root->name);

  if (mkdir(dirname, 0777) && errno != EEXIST) {
    char error_msg[MAX_STR_LEN];
    sprintf(error_msg, "Could not create dir %s", dirname);
    perror(error_msg);
    exit(1);
  }

  for (int i = 0; i < root->num_children; i++) {
    create_dir_structure(root->children[i], dirname);
  }
}

#endif
