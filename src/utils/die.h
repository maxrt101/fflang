#ifndef FF_UTILS_DIE_H_
#define FF_UTILS_DIE_H_

#include <cstdlib>

[[noreturn]] inline void die(int exit_code = EXIT_FAILURE) {
  exit(exit_code);
}

#endif

