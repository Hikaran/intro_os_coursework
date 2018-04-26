#ifndef DAG_H
#define DAG_H

#include "msg.h"

struct dag_t {};

void init_dag(struct dag_t* dag, char* dagfilepath) {
}

void handle_request(struct dag_t* dag, struct request_msg* req, struct response_msg* resp) {
  set_resp_msg(resp, "UE", "");
}

#endif
