#ifndef DAG_H
#define DAG_H

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_STR_LEN 1024

struct dag_node_t {
  char* name;
  struct dag_node_t* parent;
  struct dag_node_t** children;
  int num_children;
  int max_children;
  pthread_mutex_t mutex;
};

/** Init a node, create copy of name, set up array of children, init mutex */
struct dag_node_t* init_dag_node(char* name, int max_children) {
  struct dag_node_t* node = (struct dag_node_t*)malloc(sizeof(struct dag_node_t));
  node->name = (char*)malloc(strlen(name));
  if (name != NULL) {
    strcpy(node->name, name);
  } else {
    node->name = NULL;
  }
  node->max_children = max_children;
  node->num_children = 0;
  node->children = (struct dag_node_t**)malloc(
      max_children * sizeof(struct dag_node_t*));
  // Initialize mutex
  if (pthread_mutex_init(&(node->mutex), NULL)) {
    perror("Could not initialize mutex");
  }
  return node;
}

void free_dag(struct dag_node_t* root) {
  if (root->name != NULL) {
    free(root->name);
    root->name == NULL;
  }
  for (int i = 0; i < root->num_children; i++) {
    free_dag(root->children[i]);
  }
  free(root->children);
  root->children = NULL;
  free(root);
}

/** Attach a node to a given parent */
void add_child_node(struct dag_node_t* child, struct dag_node_t* parent) {
  // Modify child
  child->parent = parent;


  // If parent ran out of space, double the space and copy over contents
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

  // Add new child to parent
  parent->children[parent->num_children] = child;
  parent->num_children++;
}

/** Return CHILD node entry with target name or NULL if no match */
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

void create_dir_structure(struct dag_node_t* root, char* base_dir) {
  char dirname[MAX_STR_LEN];
  sprintf(dirname, "%s/%s", base_dir, root->name);
  mkdir(dirname, 0777);

  for (int i = 0; i < root->num_children; i++) {
    create_dir_structure(root->children[i], dirname);
  }
}

#endif
