#ifndef LOADLIB_WINDOWS_H
#define LOADLIB_WINDOWS_H

#define _UNICODE
#define UNICODE

#include <loadlib.h>

namespace ares {
class WindowsLibLoader : public LibLoader {
public:
    WindowsLibLoader();

    void *LoadLib(const std::wstring &filepath);
    void *LoadFunction(void *lib, const std::string &name);
};
} // namespace ares

#endif