#include <mthread_common_helpers.h>

static volatile int has_abort = 0;
void mthread_abort() { has_abort = 1; }

#define Check_has_abort()                                                      \
  {                                                                            \
    if (has_abort == 0) {                                                      \
      fprintf(stderr, "\tAbort caught\n");                                     \
      return 1;                                                                \
    } else {                                                                   \
      has_abort = 0;                                                           \
    }                                                                          \
  }                                                                            \
  (void)(0)

int main() {

  not_reachable();
  Check_has_abort();

  not_implemented();
  Check_has_abort();

  fatal_error(false);
  Check_has_abort();

  return 0;
}