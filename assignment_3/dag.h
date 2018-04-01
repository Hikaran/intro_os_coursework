#ifndef DAG_H
#define DAG_H

#include <pthread.h>

struct dag_node_t {
  char * name;
  dag_node_t * parent;
  dag_node_t * children;
  pthread_mutex_t mutex;
};

#endif
