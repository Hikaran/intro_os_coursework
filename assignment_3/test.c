#include "test_decrypt.h"
#include "test_logger.h"

int main() {
  test_decrypt_runner();
  test_logger_runner();
  printf("All tests passed!\n");
}
