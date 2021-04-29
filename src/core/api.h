#ifndef FF_CORE_API_H_
#define FF_CORE_API_H_

#define FF_API_VERSION 1

#include "version.h"
#include "core/value.h"
#include "core/object.h"

#include <string>
#include <vector>
#include <unordered_map>

#define FF_SYMBOL_EXPORT        extern "C"
#define FF_MODULE_MOD_INFO      _ff_module
#define FF_MODULE_MOD_INFO_STR  "_ff_module"

class VM;

class VMContext {
  private:
  VM* handle_;

  public:
  VMContext(VM* vm_ptr);

  void RuntimeError(const char* fmt, ...);
  void StackTrace();

  VM* GetHandle();
  int GetStackSize();
  Value Peek(int distance);
  Value GetGlobal(const std::string& name);
  const std::unordered_map<ObjString*, Value>& GetGlobals();
  std::unordered_map<std::string, ObjString*>& GetStrings();
};

struct FFModuleSymbol {
  const char* name;
  const char* doc;
  NativeFn function;
};

struct FFModuleInfo {
  const char* name;
  FFModuleSymbol* symbols;
  int symbols_length;
};

#endif

