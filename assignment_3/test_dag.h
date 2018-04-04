#ifndef TEST_DAG_H
#define TEST_DAG_H

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include "dag.h"

#define MAX_STRING_LEN 1024

void test_init_dag_node() {
  printf("test_init_dag_node() ");

  char name[] = "some node name";
  int max_children = 2;
  struct dag_node_t* node = init_dag_node(name, max_children);

  assert(strcmp(node->name, name) == 0 &&
      "node name should be the same as the given name");
  assert(node->max_children == max_children &&
      "node max children should match the given max_children");
  assert(node->num_children == 0 &&
      "node should have 0 children to start with");

  free_dag(node);

  printf("OK\n");
}

void test_add_child_node() {
  printf("test_add_child_node() ");

  int max_children = 1;

  char name_root[] = "some node name";
  struct dag_node_t* node_root = init_dag_node(name_root, max_children);
  char name_child1[] = "another node name";
  struct dag_node_t* node_child1 = init_dag_node(name_child1, max_children);
  char name_child2[] = "yet another node name";
  struct dag_node_t* node_child2 = init_dag_node(name_child2, max_children);
  char name_child3[] = "and yet another node name";
  struct dag_node_t* node_child3 = init_dag_node(name_child3, max_children);

  add_child_node(node_child1, node_root);

  assert(node_root->num_children == 1 &&
      "should have 1 child");
  assert(node_root->max_children == 1 &&
      "should not have doubled children array yet");
  node_root->children[0] = node_child1;
  assert(node_root->children[0] == node_child1 &&
      "first entry of child array should point to matching child");

  add_child_node(node_child2, node_root);

  assert(node_root->num_children == 2 &&
      "should have 2 children");
  assert(node_root->max_children == 2 &&
      "should have doubled children array once");
  assert(node_root->children[0] == node_child1 &&
      "first entry of child array should point to matching child");
  assert(node_root->children[1] == node_child2 &&
      "second entry of child array should point to matching child");

  add_child_node(node_child3, node_root);

  assert(node_root->num_children == 3 &&
      "should have 3 children");
  assert(node_root->max_children == 4 &&
      "should have had to double size of children twice");
  assert(node_root->children[0] == node_child1 &&
      "first entry of child array should point to matching child");
  assert(node_root->children[1] == node_child2 &&
      "second entry of child array should point to matching child");
  assert(node_root->children[2] == node_child3 &&
      "second entry of child array should point to matching child");

  free_dag(node_root);

  printf("OK\n");
}

void test_find_node() {
  printf("test_find_node() ");

  int max_children = 2;

  char name_root[] = "some node name";
  struct dag_node_t* node_root = init_dag_node(name_root, max_children);

  char name_child1[] = "another node name";
  struct dag_node_t* node_child1 = init_dag_node(name_child1, max_children);
  char name_child2[] = "yet another node name";
  struct dag_node_t* node_child2 = init_dag_node(name_child2, max_children);

  add_child_node(node_child1, node_root);
  add_child_node(node_child2, node_root);

  char name_grandchild1[] = "and yet another node name";
  struct dag_node_t* node_grandchild1 = init_dag_node(name_grandchild1, max_children);
  char name_grandchild2[] = "and yet another nother node name";
  struct dag_node_t* node_grandchild2 = init_dag_node(name_grandchild2, max_children);

  add_child_node(node_grandchild1, node_child1);
  add_child_node(node_grandchild2, node_child2);

  assert(find_node(node_root, name_root) == node_root &&
      "can find root node");
  assert(find_node(node_root, name_child1) == node_child1 &&
      "can find child node");
  assert(find_node(node_root, name_child2) == node_child2 &&
      "can find child node");
  assert(find_node(node_root, name_grandchild1) == node_grandchild1 &&
      "can find grandchild node");
  assert(find_node(node_root, name_grandchild2) == node_grandchild2 &&
      "can find grandchild node");

  char failquery[] = "im not a node in the dag";
  assert(find_node(node_root, failquery) == NULL &&
      "should return null for queries not in dag");

  free_dag(node_root);

  printf("OK\n");
}

void test_create_dir_structure() {
  printf("test_create_dir_structure() ");

  int max_children = 2;

  struct dag_node_t* node_root = init_dag_node("root", max_children);
  struct dag_node_t* node_child1 = init_dag_node("child1", max_children);
  struct dag_node_t* node_child2 = init_dag_node("child2", max_children);
  add_child_node(node_child1, node_root);
  add_child_node(node_child2, node_root);
  struct dag_node_t* node_grandchild1 = init_dag_node("grandchild1", max_children);
  struct dag_node_t* node_grandchild2 = init_dag_node("grandchild2", max_children);
  add_child_node(node_grandchild1, node_child1);
  add_child_node(node_grandchild2, node_child2);

  create_dir_structure(node_root, ".");

  DIR* dir = NULL;

  dir = opendir("./root");
  assert(dir != NULL &&
      "root folder should exist");
  closedir(dir);
  dir = opendir("./root/child1");
  assert(dir != NULL &&
      "root/child1 folder should exist");
  closedir(dir);
  dir = opendir("./root/child2");
  assert(dir != NULL &&
      "root/child2 folder should exist");
  closedir(dir);
  dir = opendir("./root/child3");
  assert(dir == NULL &&
      "root/child3 folder should NOT exist");
  // closedir(dir);  DO NOT CLOSE NULL ptr
  dir = opendir("./root/child1/grandchild1");
  assert(dir != NULL &&
      "root/child1/grandchild1 folder should exist");
  closedir(dir);
  dir = opendir("./root/child2/grandchild2");
  assert(dir != NULL &&
      "root/child2/grandchild2 folder should exist");
  closedir(dir);

  free(node_root);

  // USing system because removing files recusively is more work
  // than I'm willing to do for a test function
  system("rm -r ./root");

  printf("OK\n");
}

void test_parse_dag_line() {
  printf("test_parse_dag_line() ");

  int max_children = 2;

  struct dag_node_t* node_root = init_dag_node("root", max_children);
  parse_dag_line(node_root, "root:child1:child2", max_children);
  parse_dag_line(node_root, "child1:grandchild1", max_children);
  parse_dag_line(node_root, "child2:grandchild2", max_children);

  assert(node_root->num_children == 2);
  assert(strcmp(node_root->children[0]->name, "child1") == 0);
  assert(strcmp(node_root->children[1]->name, "child2") == 0);
  assert(node_root->children[0]->num_children == 1);
  assert(strcmp(node_root->children[0]->children[0]->name, "grandchild1") == 0);
  assert(node_root->children[1]->num_children == 1);
  assert(strcmp(node_root->children[1]->children[0]->name, "grandchild2") == 0);

  free_dag(node_root);

  printf("OK\n");
}

void test_parse_dag_file() {
  printf("test_parse_dag_file() ");

  int max_children = 2;
  struct dag_node_t* root = parse_dag_file("./TestCase01/input.txt", max_children);

  assert(strcmp(root->name, "Who_Won") == 0);
  assert(root->num_children == 3);
  struct dag_node_t* region1 = root->children[0];
  struct dag_node_t* region2 = root->children[1];
  struct dag_node_t* region3 = root->children[2];
  assert(region1->num_children == 1);
  assert(strcmp(region1->name, "Region_1") == 0);
  assert(region2->num_children == 0);
  assert(strcmp(region2->name, "Region_2") == 0);
  assert(region3->num_children == 0);
  assert(strcmp(region3->name, "Region_3") == 0);
  struct dag_node_t* county1 = region1->children[0];
  assert(county1->num_children == 2);
  assert(strcmp(county1->name, "County_1") == 0);
  struct dag_node_t* subcounty1 = county1->children[0];
  struct dag_node_t* subcounty2 = county1->children[1];
  assert(subcounty1->num_children == 0);
  assert(strcmp(subcounty1->name, "Sub_County_1") == 0);
  assert(subcounty2->num_children == 0);
  assert(strcmp(subcounty2->name, "Sub_County_2") == 0);

  free_dag(root);

  printf("OK\n");
}

/** Run the tests in this suite. */
void test_dag_runner() {
  test_init_dag_node();
  test_add_child_node();
  test_find_node();
  test_create_dir_structure();
  test_parse_dag_line();
  test_parse_dag_file();
}

#endif
