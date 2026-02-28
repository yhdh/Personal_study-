#include <cassert>
#include <mthread_vp_allocator.h>

int main() {
  void *ptr;
  mthread_item_buffer_allocator allocator;

  ptr = allocator.item_alloc(10);
  assert(ptr != nullptr);
  allocator.item_free(ptr);
  assert(allocator.item_alloc(0) == nullptr);

  return 0;
}