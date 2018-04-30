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
#define MSG_SIZE 256
#define MAX_LINE_LEN 4096
#define MAX_PATH_LEN 4096

struct request_msg {
  // Plus one for string terminator
  char code[CODE_LEN + 1];
  char region_name[REGION_NAME_LEN + 1];
  char data[MSG_SIZE];
};

struct response_msg {
  // Plus one for string terminator
  char code[CODE_LEN + 1];
  char data[MSG_SIZE];
};

/** Copy the strings int the msg using strcpy. */
void set_req_msg(struct request_msg* msg, char* code, char* region_name, char* data) {
  strcpy(msg->code, code);
  strcpy(msg->region_name, region_name);
  strcpy(msg->data, data);
}

/** Copy the strings int the msg using strcpy. */
void set_resp_msg(struct response_msg* msg, char* code, char* data) {
  strcpy(msg->code, code);
  strcpy(msg->data, data);
}

void req_to_str(const struct request_msg* msg, char* buf) {
  sprintf(buf, "%s;%-15s;%s", msg->code, msg->region_name, msg->data);
}

void resp_to_str(const struct response_msg* msg, char* buf) {
  sprintf(buf, "%s;%s", msg->code, msg->data);
}

/** Parse the string and populate the given struct with new string allocated
 * on heap.
 * Returns 0 if error, 1 if okay.
 * */
int parse_req_msg_str(char* msg_str, struct request_msg* msg) {
  char** segments;
  int num_segments = makeargv(msg_str, ";", &segments);
  if (num_segments != 2 && num_segments != 3) {
    fprintf(stderr,
        "Incorrect number of segments (%d) in req str (%s)\n",
        num_segments, msg_str);
    freemakeargv(segments);
    return 0;
  }
  char* code = trimwhitespace(segments[0]);
  char* region_name = trimwhitespace(segments[1]);
  if (num_segments == 3) {
    char* data = trimwhitespace(segments[2]);
    set_req_msg(msg, code, region_name, data);
  } else {
    set_req_msg(msg, code, region_name, "");
  }
  freemakeargv(segments);
  return 1;
}

/** Parse the string and populate the given struct with new string allocated
 *  on heap.
 *  Returns 0 if error, 1 if okay.
 * */
int parse_resp_msg_str(char* msg_str, struct response_msg* msg) {
  char** segments;
  int num_segments = makeargv(msg_str, ";", &segments);
  if (num_segments != 2 && num_segments != 1) {
    fprintf(stderr,
        "Incorrect number of segments (%d) in resp str (%s)\n",
        num_segments, msg_str);
    freemakeargv(segments);
    return 0;
  }
  char* code = trimwhitespace(segments[0]);
  if (num_segments == 2) {
    char* data = trimwhitespace(segments[1]);
    set_resp_msg(msg, code, data);
  } else {
    set_resp_msg(msg, code, "");
  }
  freemakeargv(segments);
  return 1;
}


/** Populate a request msg from the given line from the request file.
 *
 *  base_dir is the directory to look for any files in (for Add_Votes
 *  and Remove_Votes) DOES NOT CONTAIN TRAILING '/'
 *
 *  Return 0 if error, 1 if okay.
 */
int parse_req_file_line(char* line, struct request_msg* msg, char* base_dir) {
  char** words;
  int num_words = makeargv(line, " ", &words);
  if (num_words < 1) {
    fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    freemakeargv(words);
    return 0;
  }
  if (strcmp(words[0], "Return_Winner") == 0) {
    if (num_words != 1) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      set_req_msg(msg, "RW", "", "");
    }
  } else if (strcmp(words[0], "Count_Votes") == 0) {
    if (num_words != 2) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      set_req_msg(msg, "CV", words[1], "");
    }
  } else if (strcmp(words[0], "Open_Polls") == 0) {
    if (num_words != 2) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      set_req_msg(msg, "OP", words[1], "");
    }
  } else if (strcmp(words[0], "Add_Votes") == 0) {
    if (num_words != 3) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      char filepath[MAX_PATH_LEN];
      sprintf(filepath, "%s/%s", base_dir, words[2]);
      char data[MAX_LINE_LEN] = {'\0'};  // Make sure to init to empty str

      struct votes* results = votes_from_file(filepath);
      votes_to_string(data, results);
      set_req_msg(msg, "AV", words[1], data);
      free_votes(results);
    }
  } else if (strcmp(words[0], "Remove_Votes") == 0) {
    if (num_words != 3) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      char filepath[MAX_PATH_LEN];
      sprintf(filepath, "%s/%s", base_dir, words[2]);
      char data[MAX_LINE_LEN] = {'\0'};  // Make sure to init to empty str
      struct votes* results = votes_from_file(filepath);
      votes_to_string(data, results);
      set_req_msg(msg, "RV", words[1], data);
      free_votes(results);
    }
  } else if (strcmp(words[0], "Close_Polls") == 0) {
    if (num_words != 2) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      set_req_msg(msg, "CP", words[1], "");
    }
  } else if (strcmp(words[0], "Add_Region") == 0) {
    if (num_words != 3) {
      fprintf(stderr, "Incorrect number of params in request file line, line=%s\n", line);
    } else {
      set_req_msg(msg, "AR", words[1], words[2]);
    }
  } else {
    fprintf(stderr, "No such command found for line=%s\n", line);
  }

  freemakeargv(words);
  return msg->code != NULL;  // If msg code unset, then error occured
}

#endif
