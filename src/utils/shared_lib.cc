#include "utils/shared_lib.h"

#include <iostream>
#include <dlfcn.h>

SharedLibrary::SharedLibrary() {}

SharedLibrary::SharedLibrary(SharedLibrary&& rhs) {
  handle_ = rhs.handle_;
  rhs.handle_ = nullptr;
}

SharedLibrary::SharedLibrary(const std::string& mod_name) {
  Load(mod_name);
}

SharedLibrary::~SharedLibrary() {
  Close();
}

SharedLibrary& SharedLibrary::operator=(SharedLibrary&& rhs) {
  handle_ = rhs.handle_;
  rhs.handle_ = nullptr;
  return *this;
}

int SharedLibrary::Load(const std::string& mod_name) {
  handle_ = dlopen(mod_name.c_str(), RTLD_NOW);
  if (!handle_) {
    printf("SharedLibrary::Load: %s\n", dlerror());
    return -1;
  }

  return 0;
}

void SharedLibrary::Close() {
  if (handle_) {
    dlclose(handle_);
    handle_ = nullptr;
  }
}

void* SharedLibrary::GetSymbol(const std::string& sym_name) {
  return dlsym(handle_, sym_name.c_str());
}
