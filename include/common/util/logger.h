#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>

#define DEBUG_PRINT 0

namespace avm {

template <typename...Args>
inline void debug_log(const char *format, Args &&... args) {
#if DEBUG_PRINT
  printf(format, args...);
  printf("\n");
#endif
}

} // namespace avm

#endif
