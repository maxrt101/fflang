#include "stdlib/io.h"

#include <iostream>


Value builtin_println(int argc, Value* args) {
  for (int i = 0; i < argc; i++) {
    std::cout << args[i].ToString() << " ";
  }
  std::cout << std::endl;
  return Value(VAL_NULL);
}

Value builtin_printf(int argc, Value* args) {
  return builtin_println(argc, args);
}

