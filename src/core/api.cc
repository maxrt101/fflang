#include "core/api.h"
#include "core/vm.h"

VMContext::VMContext(VM* vm_ptr) : handle_(vm_ptr) {}

void VMContext::RuntimeError(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  handle_->vRuntimeError(fmt, args);
  va_end(args);
}

void VMContext::StackTrace() {
  handle_->StackTrace();
}

VM* VMContext::GetHandle() {
  return handle_;
}

int VMContext::GetStackSize() {
  return handle_->stack_top_ - handle_->stack_;
}

Value VMContext::Peek(int distance) {
  return handle_->Peek(distance);
}

Value VMContext::GetGlobal(const std::string& name) {
  ObjString* str = ObjString::FromStr(name);
  auto variable = handle_->globals_.find(str);
  if (variable != handle_->globals_.end()) {
    return variable->second;
  }
  return Value(VAL_NULL);
}

const std::unordered_map<ObjString*, Value>& VMContext::GetGlobals() {
  return handle_->globals_;
}

std::unordered_map<std::string, ObjString*>& VMContext::GetStrings() {
  return handle_->strings;
}
