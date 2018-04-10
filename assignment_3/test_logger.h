#ifndef TEST_LOGGER_H
#define TEST_LOGGER_H

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include "logger.h"

#define MAX_STRING_LEN 1024
#define NUM_THREADS 100

void test_logger_init_logfilename() {
  printf("test_logger_init_logfilename() ");

  char logfilename[] = "iamalogfilename.txt";
  struct logger_t logger;
  logger_init(&logger, logfilename);
  assert(strcmp(logfilename, logger.logfilename) == 0 &&
      "test_logger_init_logfilename");

  printf("OK\n");
}

struct log_msg_helper_struct {
  struct logger_t * logger;
  char msg[MAX_STRING_LEN];
};

void* log_msg_helper(void* arg) {
  struct log_msg_helper_struct* msg_struct = (struct log_msg_helper_struct*) arg;

  // Random delay
  sleep(rand() % 3);

  logger_append(msg_struct->logger, msg_struct->msg);
}

void test_logger_one_msg_per_line() {
  printf("test_logger_one_msg_per_line() ");

  char logfilename[] = "templogfile.txt";
  struct logger_t logger;
  logger_init(&logger, logfilename);

  // Remove file if it exists
  remove(logfilename);

  // Create threads.
  pthread_t pool[NUM_THREADS];
  struct log_msg_helper_struct args[NUM_THREADS];

  // Start the threads and have each log a unique message
  for (int i = 0; i < NUM_THREADS; i++) {
    args[i].logger = &logger;
    sprintf(args[i].msg, "This is message %d", i);
    pthread_create(&pool[i], NULL, log_msg_helper, (void*) &args[i]);
  }


  // Wait for threads to complete
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(pool[i], NULL);
  }

  // Check that each line in the logfile was a message
  FILE* logfile = fopen(logfilename, "r");
  char line[MAX_STRING_LEN];
  int num_lines = 0;
  while(fgets(line, MAX_STRING_LEN, logfile)) {
    num_lines++;

    // Delete trailing newline
    line[strlen(line)-1] = '\0';

    // Check for matching msg
    int has_match = 0;
    for (int j = 0; j < NUM_THREADS; j++) {
      if (strcmp(args[j].msg, line) == 0) {
        has_match = 1;
      }
    }
    assert(has_match && "Line corresponses to a msg");;
  }

  assert((num_lines == NUM_THREADS) && "Has one line for every thread");

  // Close and remove file
  fclose(logfile);
  remove(logfilename);

  printf("OK\n");
}

/** Run the tests in this suite. */
void test_logger_runner() {
  test_logger_init_logfilename();
  test_logger_one_msg_per_line();
}

#endif
