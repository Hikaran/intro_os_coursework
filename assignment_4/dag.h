#ifndef DAG_H
#define DAG_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "msg.h"
#include "votes.h"

#define MAX_REGIONS 100
#define REGION_NAME_LENGTH 16
#define MAX_LINE_LEN 4096
#define POLL_INITIAL 0
#define POLL_OPEN 1
#define POLL_CLOSED 2
#define MSG_SIZE 256

struct dag_node_t {
  char* name;
  int poll_status;
  struct votes* results;
  struct dag_node_t* parent;
  struct dag_node_t** children;
  int num_children;
};

struct region_list {
  struct dag_node_t** regions;
  int size;
};

struct dag_t {
  struct dag_node_t* root;
  struct region_list* list;
  pthread_mutex_t* mutex;
};

void append_dag_node(struct region_list* list, struct dag_node_t* node) {
  list->regions[list->size] = node;
  list->size++;
}

/**
 * Search for a DAG region node by name.
 *
 * Returns a pointer to the node if successful. Otherwise, returns NULL.
 */
struct dag_node_t* find_dag_node(struct region_list* list, char* name) {
  int i = 0;
  int list_size = list->size;

  // Search list iteratively.
  while (i < list_size) {
    if (strcmp(list->regions[i]->name, name) == 0) {
      return list->regions[i];
    }
    i++;
  }

  // No match found.
  return NULL;
}

/**
 * Initialize a DAG node with the given name.
 */
struct dag_node_t* init_dag_node(char* name) {
  // Allocate memory for node.
  struct dag_node_t* node = (struct dag_node_t*) malloc(sizeof(struct dag_node_t));

  // Allocate memory for and save region name.
  node->name = (char*) malloc(REGION_NAME_LENGTH*sizeof(char));
  strcpy(node->name, name);

  // Allocate space for child references.
  node->children = (struct dag_node_t**) calloc(MAX_REGIONS, sizeof(struct dag_node_t*));

  // Clear potential garbage values.
  node->parent = NULL;
  node->num_children = 0;
  node->poll_status = POLL_INITIAL;
  node->results = NULL;

  return node;
}

/**
 * Construct DAG and populate list of DAG regions from file.
 */
void init_dag(struct dag_t* dag, char* dagfilepath) {
  // Open DAG file.
  FILE* dag_file = fopen(dagfilepath, "r");
  if (dag_file == NULL) {
    perror("Unable to open DAG file");
    exit(1);
  }

  // Initialize DAG tree and node list.
  dag->root = NULL;
  dag->list = (struct region_list*) malloc(sizeof(struct region_list));
  dag->list->regions = (struct dag_node_t**) calloc(MAX_REGIONS, sizeof(struct dag_node_t*));
  dag->list->size = 0;

  // Initialize DAG mutex.
  dag->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  if (pthread_mutex_init(dag->mutex, NULL)) {
    perror("Could not initialize regional mutex");
    exit(1);
  }

  char line[MAX_LINE_LEN];
  errno = 0;

  while (fgets(line, MAX_LINE_LEN, dag_file)) {
    // Skip empty lines.
    trimwhitespace(line);
    if (isspace(line[0])) {
        continue;
    }

    char** node_names;
    int num_nodes = makeargv(line, ":", &node_names);

    // Each line should have a parent region and at least one child region.
    if (num_nodes < 2) {
      printf("Formatting error in DAG file: %s\n", line);
      exit(1);
    }

    struct dag_node_t* parent;
    struct dag_node_t* child;

    // Fetch parent node.
    if (dag->root == NULL) {
      // Root is uninitialized; treat parent as root.
      dag->root = init_dag_node(trimwhitespace(node_names[0]));
      append_dag_node(dag->list, dag->root);
      parent = dag->root;
    } else {
      parent = find_dag_node(dag->list, trimwhitespace(node_names[0]));
      if (parent == NULL) {
        printf("Parent node %s not found during graph construction.\n", node_names[0]);
        exit(1);
      }
    }

    // Link parent with each child node.
    for (int i = 1; i < num_nodes; i++) {
      // Initialize new child node.
      child = init_dag_node(trimwhitespace(node_names[i]));

      // Link nodes.
      child->parent = parent;
      parent->children[parent->num_children] = child;
      parent->num_children++;

      // Add child to list of nodes.
      append_dag_node(dag->list, child);
    }

    freemakeargv(node_names);
  }

  if (errno) {
    perror("Unexpected issue with DAG file");
    exit(1);
  }

  if (dag->root == NULL) {
    printf("Could not construct graph from input file.");
    exit(1);
  }

  fclose(dag_file);
}

void open_poll(struct dag_node_t* node) {
  node->poll_status = POLL_OPEN;

  for (int i = 0; i < node->num_children; i++) {
    open_poll(node->children[i]);
  }
}

void close_poll(struct dag_node_t* node) {
  node->poll_status = POLL_CLOSED;

  for (int i = 0; i < node->num_children; i++) {
    close_poll(node->children[i]);
  }
}

/**
 * Count votes from string of form "A:7,B:2,C:3"
 */
struct votes* count_new_votes(char* data) {
  // Split data by candidate.
  char** candidates;
  int num_candidates = makeargv(trimwhitespace(data), ",", &candidates);

  struct votes* new_votes = NULL;

  // Parse new votes.
  for (int i = 0; i < num_candidates; i++) {
    // Split each data segment into identity and votes.
    char** vote_info;
    if (makeargv(trimwhitespace(candidates[i]), ":", &vote_info) != 2) {
      // Formatting error detected.
      free_votes(new_votes);
      return NULL;
    }

    char* name = trimwhitespace(vote_info[0]);
    int quantity = atoi(trimwhitespace(vote_info[1]));

    if (new_votes == NULL) {
      new_votes = add_votes(new_votes, name, quantity);
    } else {
      add_votes(new_votes, name, quantity);
    }

    freemakeargv(vote_info);
  }

  freemakeargv(candidates);
  return new_votes;
}

void handle_request(struct dag_t* dag, struct request_msg* req, struct response_msg* resp) {
  // Default message.
  set_resp_msg(resp, "UE", "");

  if (strcmp(req->code, "RW") == 0) {
    // Lock tree.
    pthread_mutex_lock(dag->mutex);

    // Check if voting has concluded.
    struct dag_node_t* root = dag->root;
    if (root->poll_status == POLL_INITIAL) {
      set_resp_msg(resp, "RC", root->name);
    } else if (root->poll_status == POLL_OPEN) {
      set_resp_msg(resp, "RO", root->name);
    } else if (root->poll_status == POLL_CLOSED) {
      // Check if votes have been recorded.
      if (root->results == NULL) {
        set_resp_msg(resp, "SC", "No votes.");
        pthread_mutex_unlock(dag->mutex);
        return;
      }

      // Determine winner.
      struct votes* viewer = root->results;
      char leader[MSG_SIZE];
      int high = -1;
      while (viewer != NULL) {
        if (viewer->votes > high) {
          high = viewer->votes;
          strcpy(leader, viewer->candidate);
        }
        viewer = viewer->next;
      }

      // Construct and send response.
      char response[MSG_SIZE];
      sprintf(response, "Winner:%s", leader);
      set_resp_msg(resp, "SC", response);
    }

    // Unlock tree.
    pthread_mutex_unlock(dag->mutex);
  } else if (strcmp(req->code, "CV") == 0) {
    struct dag_node_t* node = find_dag_node(dag->list, req->region_name);
    if (node == NULL) {
      set_resp_msg(resp, "NR", req->region_name);
    } else {
      // Lock tree.
      pthread_mutex_lock(dag->mutex);

      // Check if votes have been recorded.
      if (node->results == NULL) {
        set_resp_msg(resp, "SC", "No votes.");
        pthread_mutex_unlock(dag->mutex);
        return;
      }

      struct votes* counter = node->results;

      // Record votes.
      char buf[MSG_SIZE];
      while (counter != NULL) {
        if (strlen(buf) == 0) {
          sprintf(buf, "%s:%d", counter->candidate, counter->votes);
        } else {
          sprintf(buf + strlen(buf), ",%s:%d", counter->candidate, counter->votes);
        }
        counter = counter->next;
      }

      set_resp_msg(resp, "SC", buf);

      // Unlock tree.
      pthread_mutex_unlock(dag->mutex);
    }
  } else if (strcmp(req->code, "OP") == 0) {
    struct dag_node_t* node = find_dag_node(dag->list, req->region_name);
    if (node == NULL) {
      set_resp_msg(resp, "NR", req->region_name);
    } else {
      // Lock tree.
      pthread_mutex_lock(dag->mutex);

      if (node->poll_status == POLL_OPEN) {
        char msg[MSG_SIZE];
        sprintf(msg, "%s open.", node->name);
        set_resp_msg(resp, "PF", msg);
      } else if (node->poll_status == POLL_CLOSED) {
        set_resp_msg(resp, "RR", node->name);
      } else if (node->poll_status == POLL_INITIAL) {
        open_poll(node);

        set_resp_msg(resp, "SC", "");
      }

      // Unlock tree.
      pthread_mutex_unlock(dag->mutex);
    }
  } else if (strcmp(req->code, "AV") == 0) {
    struct dag_node_t* node = find_dag_node(dag->list, req->region_name);
    if (node == NULL) {
      set_resp_msg(resp, "NR", req->region_name);
    } else {
      // Lock tree.
      pthread_mutex_lock(dag->mutex);

      // Check validity of region.
      if (node->num_children != 0) {
        set_resp_msg(resp, "NL", req->region_name);
      } else if (node->poll_status != POLL_OPEN) {
        set_resp_msg(resp, "RC", req->region_name);
      } else {
        // Count new votes.
        struct votes* tally = count_new_votes(req->data);

        if (tally == NULL) {
          pthread_mutex_unlock(dag->mutex);
          return;
        }

        // Add new votes to results.
        struct dag_node_t* region = node;
        struct votes* counter = tally;
        while (region != NULL) {
          counter = tally;
          while (counter != NULL) {
            if (region->results == NULL) {
              region->results = add_votes(region->results, counter->candidate, counter->votes);
            } else {
              add_votes(region->results, counter->candidate, counter->votes);
            }
            counter = counter->next;
          }
          region = region->parent;
        }
        free_votes(tally);
        set_resp_msg(resp, "SC", "");
      }

      // Unlock tree.
      pthread_mutex_unlock(dag->mutex);
    }
  } else if (strcmp(req->code, "RV") == 0) {
    struct dag_node_t* node = find_dag_node(dag->list, req->region_name);
    if (node == NULL) {
      set_resp_msg(resp, "NR", req->region_name);
    } else {
      // Lock tree.
      pthread_mutex_lock(dag->mutex);

      // TODO
      if (node->num_children != 0) {
        set_resp_msg(resp, "NL", req->region_name);
      } else if (node->poll_status != POLL_OPEN) {
        set_resp_msg(resp, "RC", req->region_name);
      } else {
        struct votes* tally = count_new_votes(req->data);
        if (tally != NULL) {
          free_votes(tally);
        }

        // TODO
      }

      // Unlock tree.
      pthread_mutex_unlock(dag->mutex);
    }
  } else if (strcmp(req->code, "CP") == 0) {
    struct dag_node_t* node = find_dag_node(dag->list, req->region_name);
    if (node == NULL) {
      set_resp_msg(resp, "NR", req->region_name);
    } else {
      // Lock tree.
      pthread_mutex_lock(dag->mutex);

      if (node->poll_status == POLL_INITIAL) {
        char msg[MSG_SIZE];
        sprintf(msg, "%s unopened.", node->name);
        set_resp_msg(resp, "PF", msg);
      } else if (node->poll_status == POLL_CLOSED) {
        char msg[MSG_SIZE];
        sprintf(msg, "%s closed.", node->name);
        set_resp_msg(resp, "PF", msg);
      } else if (node->poll_status == POLL_OPEN) {
        close_poll(node);

        set_resp_msg(resp, "SC", "");
      }

      // Unlock tree.
      pthread_mutex_unlock(dag->mutex);
    }
  } else {
    set_resp_msg(resp, "UC", req->code);
  }
}

#endif
