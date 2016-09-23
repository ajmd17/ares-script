#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>

#define DEBUG_PRINT_ENABLED 0

namespace avm {
#if DEBUG_PRINT_ENABLED
#define DEBUG_LOG(str, ...) printf("0x%08x: " str "\n", (int)state->stream->Position(), __VA_ARGS__)
#else
#define DEBUG_LOG (void)0;
#endif
} // namespace avm

#endif
