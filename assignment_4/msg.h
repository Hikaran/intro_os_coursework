#ifndef MSG_H
#define MSG_H

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "makeargv.h"
#include "util.h"
#include "votes.h"

#define CODE_LEN 2
#define REGION_NAME_LEN 15
#define MAX_LINE_LEN 4096

struct request_msg {
  // None should be NULL
  char* code;
  char* region_name;
  char* data;
};

struct response_msg {
  // None should be NULL
  char* code;
  char* data;
};

/** Init msg by creating space on heap and copying strings over
 *  All fields of msg should be NULL.
 *  Returns 0 if error, 1 if okay.
 */
int init_request_msg(struct request_msg* msg, char* code, char* region_name, char* data) {
  if (code == NULL || strlen(code) != CODE_LEN) {
    fprintf(stderr, "Code not given or incorrect size, code=%s\n", code);
    return 0;
  }
  if (region_name == NULL && strlen(region_name) > REGION_NAME_LEN) {
    fprintf(stderr, "Region name is too long, region_name=%s\n", region_name);
    return 0;
  }
  msg->code = (char*)malloc((CODE_LEN + 1) * sizeof(char));
  sprintf(msg->code, "%s", code);
  msg->region_name = (char*)malloc((REGION_NAME_LEN+1) * sizeof(char));
  if (region_name == NULL) {
    sprintf(msg->region_name, "%s", "");
  } else {
    sprintf(msg->region_name, "%s", region_name);
  }
  if (data == NULL) {
    msg->data = (char*)malloc(1 * sizeof(char));
    sprintf(msg->data, "%s", "");
  } else {
    msg->data = (char*)malloc((strlen(data) + 1) * sizeof(char));
    sprintf(msg->data, "%s", data);
  }
  return 1;
}

/** Free all fields in msg and set to NULL */
void free_request_msg_fields(struct request_msg* msg) {
  if (msg->code != NULL) {
    free(msg->code);
    msg->code = NULL;
  }
  if (msg->region_name != NULL) {
    free(msg->region_name);
    msg->region_name = NULL;
  }
  if (msg->data != NULL) {
    free(msg->data);
    msg->data = NULL;
  }
}

/** Init msg by creating space on heap and copying strings over
 *  All fields of msg should be NULL.
 *  Returns 0 if error, 1 if okay.
 */
int init_response_msg(struct response_msg* msg, char* code, char* data) {
  if (code == NULL || strlen(code) != CODE_LEN) {
    fprintf(stderr, "Code not given or is not correct size, code=%s\n", code);
    return 0;
  }
  msg->code = (char*)malloc((CODE_LEN + 1) * sizeof(char));
  sprintf(msg->code, "%s", msg);
  if (data == NULL) {
    msg->data = (char*)malloc(1 * sizeof(char));
    sprintf(msg->data, "%s", "");
  } else {
    msg->data = (char*)malloc((strlen(data) + 1) * sizeof(char));
    sprintf(msg->data, "%s", data);
  }
  return 1;
}

/** Free all fields in msg and set to NULL */
void free_response_msg_fields(struct response_msg* msg) {
  if (msg->code != NULL) {
    free(msg->code);
    msg->code = NULL;
  }
  if (msg->data != NULL) {
    free(msg->data);
    msg->data = NULL;
  }
}

/* Return 0 if invalid, 1 if valid. */
int valid_request_msg(const struct request_msg* msg) {
  if (msg == NULL) {
    fprintf(stderr, "Request msg can not be NULL\n");
    return 0;
  }
  if (msg->code == NULL) {
    fprintf(stderr, "Request msg code cannot be NULL\n");
    return 0;
  }
  if (msg->region_name == NULL) {
    fprintf(stderr, "Request msg region name cannot be NULL\n");
    return 0;
  }
  if (msg->data == NULL) {
    fprintf(stderr, "Request msg data cannot be NULL\n");
    return 0;
  }
  if (strlen(msg->code) != CODE_LEN) {
    fprintf(stderr, "Request msg code is wrong size, code=%s\n", msg->code);
    return 0;
  }
  if (strlen(msg->region_name) > REGION_NAME_LEN) {
    fprintf(stderr, "Request msg region name too long, region_name=%s\n", msg->region_name);
    return 0;
  }
  return 1;
}

/* Return 0 if invalid, 1 if valid. */
int valid_response_msg(const struct response_msg* msg) {
  if (msg == NULL) {
    fprintf(stderr, "Request msg can not be NULL\n");
    return 0;
  }
  if (msg->code == NULL) {
    fprintf(stderr, "Request msg code cannot be NULL\n");
    return 0;
  }
  if (msg->data == NULL) {
    fprintf(stderr, "Request msg data cannot be NULL\n");
    return 0;
  }
  if (strlen(msg->code) != CODE_LEN) {
    fprintf(stderr, "Request msg code is wrong size, code=%s\n", msg->code);
    return 0;
  }
  return 1;
}

/** Return a new string allocated on heap of the given request. */
char* request_to_string(const struct request_msg* msg) {
  // Validate so don't have to worry about NULL values
  if (!valid_request_msg(msg)) {
    return NULL;
  }
  int msg_len = CODE_LEN + REGION_NAME_LEN + strlen(msg->data);
  msg_len += 3;  // Two semicolons and a null terminator
  char* msg_str = (char*)malloc(msg_len * sizeof(char));
  sprintf(msg_str, "%s;%-15s;%s", msg->code, msg->region_name, msg->data);
  return msg_str;
}

/** Return a new string allocated on heap of the given request. */
char* response_to_string(const struct response_msg* msg) {
  // Validate so don't have to worry about NULL values
  if (!valid_response_msg(msg)) {
    return NULL;
  }
  int msg_len = CODE_LEN + strlen(msg->data);
  msg_len += 2;  // One semicolons and a null terminator
  char* msg_str = (char*)malloc(msg_len * sizeof(char));
  sprintf(msg_str, "%s;%s", msg->code, msg->data);
  return msg_str;
}

/** Parse the string and populate the given struct with new string allocated
 * on heap.
 * Returns 0 if error, 1 if okay.
 * */
int parse_request_msg_string(char* msg_str, struct request_msg* msg) {
  char** segments;
  int num_segments = makeargv(msg_str, ";", &segments);
  if (num_segments != 3) {
    fprintf(stderr, "Incorrect number of segments in request str, str=%s\n", msg_str);
    freemakeargv(segments);
    return 0;
  }
  char* code = trimwhitespace(segments[0]);
  char* region_name = trimwhitespace(segments[1]);
  char* data = trimwhitespace(segments[3]);
  free_request_msg_fields(msg);
  init_request_msg(msg, code, region_name, data);
  if (!valid_request_msg(msg)) {
    freemakeargv(segments);
    free_request_msg_fields(msg);
    return 0;
  }
  return 1;
}

/** Parse the string and populate the given struct with new string allocated
 *  on heap.
 *  Returns 0 if error, 1 if okay.
 * */
int parse_respose_msg_string(char* msg_str, struct response_msg* msg) {
  char** segments;
  int num_segments = makeargv(msg_str, ";", &segments);
  if (num_segments != 2) {
    fprintf(stderr, "Incorrect number of segments in request str, str=%s\n", msg_str);
    freemakeargv(segments);
    return 0;
  }
  char* code = trimwhitespace(segments[0]);
  char* data = trimwhitespace(segments[3]);
  free_response_msg_fields(msg);
  init_response_msg(msg, code, data);
  if (!valid_response_msg(msg)) {
    freemakeargv(segments);
    free_response_msg_fields(msg);
    return 0;
  }
  return 1;
}


/** Populate a request msg from the given line from the request file.
 *  Return 0 if error, 1 if okay.
 */
int parse_request_file_line(char* line, struct request_msg* msg) {
  char** words;
  int num_words = makeargv(line, " ", &words);
  if (num_words < 1) {
    fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    freemakeargv(words);
    return 0;
  }
  free_request_msg_fields(msg);
  if (strcmp(words[0], "Return_Winner") == 0) {
    if (num_words != 1) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      init_request_msg(msg, "RW", "", "");
    }
  } else if (strcmp(words[0], "Count_Votes") == 0) {
    if (num_words != 2) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      init_request_msg(msg, "CV", words[1], "");
    }
  } else if (strcmp(words[0], "Open_Polls") == 0) {
    if (num_words != 2) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      init_request_msg(msg, "OP", words[1], "");
    }
  } else if (strcmp(words[0], "Add_Votes") == 0) {
    if (num_words != 3) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      char data[MAX_LINE_LEN] = {'\0'};  // Make sure to init to empty str
      struct votes* results = votes_from_file(words[2]);
      votes_to_string(data, results);
      init_request_msg(msg, "AV", words[1], data);
      free_votes(results);
    }
  } else if (strcmp(words[0], "Remove_Votes") == 0) {
    if (num_words != 3) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      char data[MAX_LINE_LEN] = {'\0'};  // Make sure to init to empty str
      struct votes* results = votes_from_file(words[2]);
      votes_to_string(data, results);
      init_request_msg(msg, "RV", words[1], data);
      free_votes(results);
    }
  } else if (strcmp(words[0], "Close_Polls") == 0) {
    if (num_words != 2) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      init_request_msg(msg, "CP", words[1], "");
    }
  } else if (strcmp(words[0], "Add_Region") == 0) {
    if (num_words != 3) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      init_request_msg(msg, "AR", words[1], words[2]);
    }
  } else {
    fprintf(stderr, "No such command found for line=%s\n", line);
  }

  freemakeargv(words);
  return msg->code != NULL;  // If msg code unset, then error occured
}

#endif
