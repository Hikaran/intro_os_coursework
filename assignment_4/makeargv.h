#ifndef MAKEARGV_H
#define MAKEARGV_H

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**********************************
* Taken from Unix Systems Programming, Robbins & Robbins, p37
*********************************/
int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;
   char *saveptr;

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
   if (strtok_r(t, delimiters, &saveptr) != NULL)
      for (numtokens = 1; strtok_r(NULL, delimiters, &saveptr) != NULL; numtokens++) ;

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
      **argvp = strtok_r(t,delimiters,&saveptr);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok_r(NULL,delimiters,&saveptr);
   }
   *((*argvp) + numtokens) = NULL;
   return numtokens;
}

/**********************************
* Taken from Unix Systems Programming, Robbins & Robbins, p38
*********************************/
void freemakeargv(char **argv) {
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}

#endif
