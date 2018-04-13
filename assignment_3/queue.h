#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFFSET 2
#define WRAPLENGTH 26

struct queue_node_t {
  char* value;
  struct queue_node_t* next;
};

struct queue_t {
  struct queue_node_t* root;
  pthread_mutex_t mutex;
};

/**
 * Initializes a queue of defined type struct queue_t.
 *
 * Takes in a pointer to the queue to be initialized.
 */
void init_queue(struct queue_t* queue) {
  if (queue == NULL) {
    fprintf(stderr, "queue cannot be NULL\n");
    exit(1);
  }
  // Set up dummy header node.
  queue->root = (struct queue_node_t*)malloc(sizeof(struct queue_node_t));
  queue->root->value = NULL;
  queue->root->next = NULL;

  // Initialize mutex.
  if (pthread_mutex_init(&(queue->mutex), NULL)) {
    perror("Could not inialize mutex");
    exit(1);
  }
}

/** 
 * Free all contents of queue of defined type struct queue_t.
 *
 * Does not free the memory allocated for queue.
 */
void free_queue(struct queue_t* queue) {
  if (queue == NULL) {
    return;
  }
  // Free nodes and values
  struct queue_node_t* node = queue->root;
  while (node != NULL) {
    // Free the current node, then move on to the next
    struct queue_node_t* next_node = node->next;
    if (node->value != NULL) {
      free(node->value);
    }
    free(node);
    node = next_node;
  }
  // Clean up queue
  pthread_mutex_destroy(&(queue->mutex));
}

/** 
 * Enqueue value as a node at the end of the queue.
 */
void enqueue(struct queue_t* queue, char* value) {
  if (queue == NULL) {
    fprintf(stderr, "queue cannot be NULL\n");
    exit(1);
  }
  if (value == NULL) {
    fprintf(stderr, "value to enqueue cannot be NULL\n");
    exit(1);
  }
  pthread_mutex_lock(&(queue->mutex));

  struct queue_node_t* node = queue->root;
  // Set node to the last node in the list
  while(node->next != NULL) {
    node = node->next;
  }
  // Append new node with given value to end of list
  node->next = (struct queue_node_t*)malloc(sizeof(struct queue_node_t));
  node->next->next = NULL;
  node->next->value = (char*)malloc(strlen(value));
  strcpy(node->next->value, value);

  pthread_mutex_unlock(&(queue->mutex));
}

/**
 * Dequeue a value from the front of the queue and copy it to the char
 * array provided as the second parameter.
 */
void dequeue(struct queue_t* queue, char* val) {
  if (queue == NULL) {
    fprintf(stderr, "queue cannot be NULL\n");
    exit(1);
  }
  pthread_mutex_lock(&(queue->mutex));

  struct queue_node_t* node = queue->root;
  // Early return if nothing to dequeue
  if (node->next == NULL) {
    return;
  }
  // Dequeue the front node
  struct queue_node_t* dequeued_node = node->next;
  node->next = dequeued_node->next;

  // Copy the value of the dequeued node over
  if (val != NULL) {
    strcpy(val, dequeued_node->value);
  }

  free(dequeued_node->value);
  free(dequeued_node);

  pthread_mutex_unlock(&(queue->mutex));
}

#endif
