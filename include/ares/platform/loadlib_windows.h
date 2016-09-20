#ifndef LOADLIB_WINDOWS_H
#define LOADLIB_WINDOWS_H

#include <loadlib.h>

namespace ares {
class WindowsLibLoader : public LibLoader {
public:
    void *LoadLib(const std::wstring &filepath);
    void *LoadFunction(void *lib, const std::string &name);
};
} // namespace ares

#endif