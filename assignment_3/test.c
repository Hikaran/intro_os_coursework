#include "test_dag.h"
#include "test_decrypt.h"
#include "test_logger.h"
#include "test_makeargv.h"
#include "test_queue.h"

int main() {
  test_dag_runner();
  test_decrypt_runner();
  test_logger_runner();
  test_makeargv_runner();
  test_queue_runner();
  printf("All tests passed!\n");
}
