#ifndef FF_CORE_MODULE_H_
#define FF_CORE_MODULE_H_

#include "utils/shared_lib.h"
#include "core/api.h"

#include <vector>

class FFModule {
 private:
  SharedLibrary mod_lib_;
  FFModuleInfo* mod_info_ = nullptr;
  std::vector<FFModuleSymbol> symbols_;

 public:
  FFModule();
  FFModule(const std::string& lib_name);
  FFModule(FFModule&& rhs);
  FFModule(const FFModule& rhs) = delete;
  ~FFModule();

  int Load(const std::string& lib_name);
  FFModuleSymbol* GetSymbol(const char* symbol_name);
  std::vector<FFModuleSymbol>& GetAllSymbols();
};

#endif