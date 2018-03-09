#ifndef UTIL_H
#define UTIL_H

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The maximum amount of bytes for a file name */
#define MAX_FILE_NAME_SIZE 255

/* The maximum amount of bytes for each I/O operation */
#define MAX_IO_BUFFER_SIZE 1024

/**********************************
* 
* Taken from Unix Systems Programming, Robbins & Robbins, p37
* 
*********************************/
int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);
   if ((t = malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t,snew);
   numtokens = 0;
   if (strtok(t, delimiters) != NULL)
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

   if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }

   if (numtokens == 0)
      free(t);
   else {
      strcpy(t,snew);
      **argvp = strtok(t,delimiters);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok(NULL,delimiters);
   }
   *((*argvp) + numtokens) = NULL;
   return numtokens;
}

/**********************************
* 
* Taken from Unix Systems Programming, Robbins & Robbins, p38
* 
*********************************/
void freemakeargv(char **argv) {
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}

char *trimwhitespace(char *str) {
  char *end;
  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;

  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

/** Split str by sep and copy last result into buf. */
void put_last_seperator(char* buf, char* str, char* sep) {
    char ***splits = malloc(strlen(str)*sizeof(char));
    int num_items = makeargv(str, sep, splits);
    strcpy(buf, (*splits)[num_items-1]);
    free(splits);
}

/** Wait for all child proccess to finish running. */
void wait_for_all_children() {
  while (1) {
    errno = 0;  // Reset errno
    int status;
    int wait_pid = wait(&status);
    if (wait_pid == -1) {
      // No more children can exit loop
      if (errno == ECHILD) {
        break;
      }
    } else {
      if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        printf("Child process exited abnormally.\n");
        printf("Exit status = %d\n", WEXITSTATUS(status));
        exit(WEXITSTATUS(status));
      }
    }
  }
}

#endif
