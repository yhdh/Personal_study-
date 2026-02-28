#include <cstdlib>
#include <unistd.h>

static volatile int has_abort = 0;
void abort(void) {
  has_abort = 1;
  while (has_abort == 1)
    sleep(1);
  exit(1);
}

[[maybe_unused]] static void wait_abort() {
  while (has_abort == 0)
    sleep(1);
}
