#include <mthread_vp_allocator.h>

#define BUFFER_SIZE 10

int main() {
  mthread_item_buffer_allocator allocator(BUFFER_SIZE);
  void *ptr;

  ptr = allocator.item_alloc(BUFFER_SIZE);
  allocator.item_free(ptr);

  return 0;
}