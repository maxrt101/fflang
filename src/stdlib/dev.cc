#include "ff.h"

#include <iostream>

Value dev_print_globals(void* ctx, int argc, Value* args) {
  VMContext* context = (VMContext*)ctx;

  if (context) {
    auto& globals = context->GetGlobals();
    for (auto& global : globals) {
      std::cout << global.first->str << " = " << global.second.ToString() << "\n";
    }
    return Value(true);
  }

  return Value(false);
}

Value dev_print_stack(void* ctx, int argc, Value* args) {
  VMContext* context = (VMContext*)ctx;

  if (context) {
    for (int i = 0; i < context->GetStackSize(); i++) {
      std::cout << context->Peek(i).ToString() << "\n";
    }
    return Value(true);
  }

  return Value(false);
}


FFModuleSymbol symbols[] {
  {"print_globals", "", dev_print_globals},
  {"print_stack", "", dev_print_stack}
};

FF_SYMBOL_EXPORT FFModuleInfo FF_MODULE_MOD_INFO {
  "dev",
  symbols,
  2
};