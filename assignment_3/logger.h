#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_LENGTH 1024

struct logger_t {
  char logfilename[MAX_FILE_LENGTH];
  pthread_mutex_t mutex;
};

/** Initalize the given logger. */
void logger_init(struct logger_t* logger, char* logfilename) {
  strcpy(logger->logfilename, logfilename);

  // Initalize mutex
  if (pthread_mutex_init(&(logger->mutex), NULL)) {
    perror("Could not inialize mutex");
    exit(1);
  }
}

void logger_append(struct logger_t* logger, char* msg) {
  pthread_mutex_lock(&(logger->mutex));

  FILE* logfile = fopen(logger->logfilename, "a");
  if (logfile == NULL) {
    perror("Could not create or append to logfile");
    exit(1);
  }
  fprintf(logfile, "%s\n", msg);
  fclose(logfile);

  pthread_mutex_unlock(&(logger->mutex));
}

#endif
