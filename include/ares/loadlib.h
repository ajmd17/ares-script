#ifndef LOADLIB_H
#define LOADLIB_H

#include <string>

namespace ares {
class LibLoader {
public:
  virtual void *LoadLib(const std::wstring &filepath) = 0;
  virtual void *LoadFunction(void *lib, const std::string &name) = 0;
};
} // namespace ares

#endif