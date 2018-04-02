#include "test_decrypt.h"
#include "test_dag.h"

int main() {
  test_decrypt_runner();
  test_dag_runner();
  printf("All tests passed!\n");
}
