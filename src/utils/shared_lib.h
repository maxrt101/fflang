#ifndef FF_UTILS_SHARED_LIB_H_
#define FF_UTILS_SHARED_LIB_H_

#include <string>

class SharedLibrary {
 private:
  void* handle_ = nullptr;

 public:
  SharedLibrary();
  SharedLibrary(const std::string& mod_name);
  SharedLibrary(SharedLibrary&& rhs);
  SharedLibrary(const SharedLibrary& rhs) = delete;
  ~SharedLibrary();

  SharedLibrary& operator=(SharedLibrary&& rhs);

  int Load(const std::string& mod_name);
  void Close();
  void* GetSymbol(const std::string& sym_name);
};

#endif