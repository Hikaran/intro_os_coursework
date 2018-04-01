#include "test_decrypt.h"
#include "test_queue.h"

int main() {
  test_decrypt_runner();
  test_queue_runner();
  printf("All tests passed!\n");
}
