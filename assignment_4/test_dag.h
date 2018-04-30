#ifndef TEST_DAG_H
#define TEST_DAG_H

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "dag.h"

#define MAX_STRING_LEN 1024

void test_init_dag_node() {
  printf("Testing node initialization.\n");

  char name[] = "some node name";
  struct dag_node_t* node = init_dag_node(name);

  assert(strcmp(node->name, name) == 0 &&
      "node name should be the same as the given name");
  assert(node->num_children == 0 &&
      "node should have 0 children to start with");
  assert(node->poll_status == POLL_INITIAL &&
      "node should have a closed poll");
  assert(node->results == NULL &&
      "node should have NULL pointer for results");
  assert(node->parent == NULL &&
      "node should have NULL pointer for parent");

  printf("Passed!\n");
}

void test_init_dag() {
  printf("Testing dag initialization.\n");
  struct dag_t dag;
  init_dag(&dag, "./TestCases/TestCase01/voting_regions.dag");

  struct dag_node_t* root = dag.root;
  assert(strcmp(root->name, "Who_Won") == 0);
  assert(root->num_children == 2);
  struct dag_node_t* region1 = root->children[0];
  struct dag_node_t* region2 = root->children[1];
  assert(strcmp(region1->name, "Region_1") == 0);
  assert(region1->num_children == 2);
  assert(strcmp(region2->name, "Region_2") == 0);
  assert(region2->num_children == 0);
  struct dag_node_t* county1 = region1->children[0];
  struct dag_node_t* county2 = region1->children[1];
  assert(strcmp(county1->name, "County_1") == 0);
  assert(county1->num_children == 0);
  assert(strcmp(county2->name, "County_2") == 0);
  assert(county2->num_children == 0);

  printf("Passed!\n");
}

/** Run the tests in this suite. */
void test_dag_runner() {
  test_init_dag_node();
  test_init_dag();
}

#endif
