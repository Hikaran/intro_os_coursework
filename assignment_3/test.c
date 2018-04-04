#include "test_decrypt.h"
#include "test_dag.h"
#include "test_makeargv.h"

int main() {
  test_decrypt_runner();
  test_dag_runner();
  test_makeargv_runner();
  printf("All tests passed!\n");
}
