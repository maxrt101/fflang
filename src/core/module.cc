#include "core/module.h"

#include <algorithm>
#include <iostream>
#include <cstring>

FFModule::FFModule() {}

FFModule::FFModule(const std::string& lib_name) {
  Load(lib_name);
}

FFModule::FFModule(FFModule&& rhs) {
  mod_lib_ = std::move(rhs.mod_lib_);
  mod_info_ = rhs.mod_info_;
  symbols_ = std::move(rhs.symbols_);
}

FFModule::~FFModule() {}

int FFModule::Load(const std::string& lib_name) {
  int lib_load_ret = mod_lib_.Load(lib_name);
  if (lib_load_ret != 0) {
    fprintf(stderr, "Failed to load module '%s' (%d)\n", lib_name.c_str(), lib_load_ret);
    return lib_load_ret;
  }

  mod_info_ = (FFModuleInfo*)mod_lib_.GetSymbol(FF_MODULE_MOD_INFO_STR);

  if (!mod_info_) {
    fprintf(stderr, "Falied to load module '%s': No module info found\n", lib_name.c_str());
    return -1;
  }

  for (int i = 0; i < mod_info_->symbols_length; i++) {
    symbols_.push_back(mod_info_->symbols[i]);
  }

  return 0;
}

FFModuleSymbol* FFModule::GetSymbol(const char* symbol_name) {
  auto itr = std::find_if(symbols_.begin(), symbols_.end(),
      [&](auto& element) { return strcmp(element.name, symbol_name) == 0; });
  
  return (itr == symbols_.end()) ? nullptr : &(*itr);
}

std::vector<FFModuleSymbol>& FFModule::GetAllSymbols() {
  return symbols_;
}
