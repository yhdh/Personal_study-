#include <mthread.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_yield();
  return 0;
}
