#define _XOPEN_SOURCE 500

#include "test_decrypt.h"
#include "test_dag.h"
#include "test_makeargv.h"
#include "test_rmrf.h"

int main() {
  test_decrypt_runner();
  test_dag_runner();
  test_makeargv_runner();
  test_rmrf_runner();
  printf("All tests passed!\n");
}
