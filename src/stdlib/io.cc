#include "ff.h"

#include <iostream>


Value builtin_println(void* ctx, int argc, Value* args) {
  for (int i = 0; i < argc; i++) {
    std::cout << args[i].ToString() << " ";
  }
  std::cout << std::endl;
  return Value(VAL_NULL);
}

Value builtin_printf(void* ctx, int argc, Value* args) {
  return builtin_println(ctx, argc, args);
}


FFModuleSymbol symbols[] {
  {"println", "", builtin_println},
  {"printf", "", builtin_printf}
};

FF_SYMBOL_EXPORT FFModuleInfo FF_MODULE_MOD_INFO {
  "io",
  symbols,
  2
};
