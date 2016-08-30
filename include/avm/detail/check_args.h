#ifndef CHECK_ARGS_H
#define CHECK_ARGS_H

#include <detail/vm_state.h>

#include <cstdint>

namespace avm {
  bool CheckArgs(VMState *state, uint32_t nreqargs, uint32_t nrecargs);
}

#endif