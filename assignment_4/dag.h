#ifndef DAG_H
#define DAG_H

#include "msg.h"


void handle_request(struct request_msg* req, struct response_msg* resp) {
  set_resp_msg(resp, "UE", "");
}

#endif
