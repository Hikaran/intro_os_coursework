#ifndef TEST_QUEUE_H
#define TEST_QUEUE_H

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "queue.h"

#define MAX_STRING_LEN 1024
#define NUM_THREADS 100

void test_queue_init_dummy_header() {
  printf("test_queue_init_dummy_header() ");

  struct queue_t queue;
  init_queue(&queue);
  assert(queue.root != NULL &&
      "test queue dummy header is not NULL");
  assert(queue.root->next == NULL &&
      "test queue dummy header dosn't point to anything");
  assert(queue.root->value == NULL &&
      "test queue dummy header should not point to anything");

  free_queue(&queue);

  printf("OK\n");
}

void test_enqueue_one_value() {
  printf("test_enqueue_one_value() ");

  struct queue_t queue;
  init_queue(&queue);

  char val[] = "Just some string value";
  enqueue(&queue, val);

  assert(queue.root->next != NULL &&
      "test queue dummy should point to a new value");
  assert(queue.root->next->next == NULL &&
      "first item of queue should be end of the list");
  assert(strcmp(queue.root->next->value, val) == 0 &&
      "first item of queue should match the enqueued val");

  free_queue(&queue);

  printf("OK\n");
}

struct enqueue_thread_struct {
  char val[MAX_STRING_LEN];
  struct queue_t* queue;
};

void* enqueue_thread_call(void* arg) {
  struct enqueue_thread_struct* arg_struct =
    (struct enqueue_thread_struct*)arg;

  // Random delay
  sleep(rand() % 3);

  enqueue(arg_struct->queue, arg_struct->val);
}

void test_enqueue_threaded() {
  printf("test_enqueue_threaded() ");

  struct queue_t queue;
  init_queue(&queue);

  // Create threads.
  pthread_t pool[NUM_THREADS];
  struct enqueue_thread_struct args[NUM_THREADS];

  // Start the threads and add enqueue each with a unique message
  for (int i = 0; i < NUM_THREADS; i++) {
    args[i].queue = &queue;
    sprintf(args[i].val, "This is message %d", i);
    pthread_create(&pool[i], NULL, enqueue_thread_call, (void*) &args[i]);
  }

  // Wait for threads to complete
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(pool[i], NULL);
  }

  // Check that there one node was created from each thread
  for (int i = 0; i < NUM_THREADS; i++) {
    int has_match = 0;
    struct queue_node_t* cur_node = queue.root;
    while(!has_match && cur_node->next != NULL) {
      cur_node = cur_node->next;
      if (strcmp(cur_node->value, args[i].val) == 0) {
        has_match = 1;
      }
    }
    assert(has_match &&
        "Each thread should have a node in the queue");
  }

  // Check that only one node per thread was created
  struct queue_node_t* cur_node = queue.root;
  int num_nodes = 0;
  while(cur_node->next != NULL) {
    num_nodes++;
    cur_node = cur_node->next;
  }
  assert(num_nodes == NUM_THREADS &&
      "Only one node in queue per thread");

  free_queue(&queue);

  printf("OK\n");
}

void test_dequeue_one_val() {
  printf("test_dequeue_one_val() ");

  struct queue_t queue;
  init_queue(&queue);
  char val[] = "Just some string value";
  enqueue(&queue, val);

  char* dequeued_val = dequeue(&queue);

  assert(strcmp(dequeued_val, val) == 0 &&
      "dequeued_val should match enqueued val");
  assert(queue.root != NULL &&
      "queue dummy header should not be NULL");
  assert(queue.root->next == NULL &&
      "queue should be empty");
  assert(queue.root->value == NULL &&
      "test queue dummy header should not point to anything");

  free_queue(&queue);
  free(dequeued_val);
  printf("OK\n");
}

void* dequeue_thread_call(void* arg) {
  struct queue_t* queue = (struct queue_t*)arg;

  // Random delay
  sleep(rand() % 3);

  char* dequeued_val = dequeue(queue);
  free(dequeued_val);  // Must remeber to free the dequeued value
}

void test_dequeue_threaded() {
  printf("test_dequeue_threaded() ");

  struct queue_t queue;
  init_queue(&queue);

  // Add nodes to the queue (one more than the number of threads)
  for (int i = 0; i < NUM_THREADS + 1; i++) {
    char val[MAX_STRING_LEN];
    sprintf(val, "Enqueued value %d", i);
    enqueue(&queue, val);
  }

  // Store value of last node to check later (should be only node left
  // after running the threads)
  char last_val[MAX_STRING_LEN];
  struct queue_node_t* last_node = queue.root;
  while(last_node->next != NULL) {
    last_node = last_node->next;
  }
  strcpy(last_val, last_node->value);

  // Create threads.
  pthread_t pool[NUM_THREADS];
  struct enqueue_thread_struct args[NUM_THREADS];

  // Start the threads
  for (int i = 0; i < NUM_THREADS; i++) {
    args[i].queue = &queue;
    pthread_create(&pool[i], NULL, dequeue_thread_call, (void*)&queue);
  }

  // Wait for threads to complete
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(pool[i], NULL);
  }

  // Check that the last node is the only node left in the queue
  struct queue_node_t* root = queue.root;
  assert(root->next != NULL &&
      "queue should not be empty");
  assert(root->next->next == NULL &&
      "queue should only have one reamining node");
  assert(root->next == last_node &&
      "last node should be the last node of the orignal queue");
  assert(strcmp(root->next->value, last_val) == 0 &&
      "last node's value should match the last node of the orginal queue");

  free_queue(&queue);

  printf("OK\n");
}

/** Run the tests in this suite. */
void test_queue_runner() {
  test_queue_init_dummy_header();
  test_enqueue_one_value();
  test_enqueue_threaded();
  test_dequeue_one_val();
  test_dequeue_threaded();
}

#endif
