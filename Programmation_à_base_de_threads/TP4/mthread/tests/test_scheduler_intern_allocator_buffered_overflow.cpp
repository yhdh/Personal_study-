#include <mthread_vp_allocator.h>

#define BUFFER_SIZE 10
#define BLOCK_NUMBER (MTHREAD_BLOCK_NUMBER * 2)

int main() {
  mthread_item_buffer_allocator allocator(BUFFER_SIZE);
  void *ptr[BLOCK_NUMBER];

  for (auto &i : ptr) {
    i = allocator.item_alloc(BUFFER_SIZE);
  }
  for (auto &i : ptr) {
    allocator.item_free(i);
  }

  return 0;
}