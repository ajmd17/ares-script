#include <platform/loadlib_windows.h>
#include <windows.h>

namespace ares {
void *WindowsLibLoader::LoadLib(const std::wstring &filepath) {
  HINSTANCE lib = LoadLibrary(filepath.c_str());
  return lib;
}

void *WindowsLibLoader::LoadFunction(void *lib, const std::string &name) {
  return GetProcAddress((HINSTANCE)lib, name.c_str());
}
} // namespace ares