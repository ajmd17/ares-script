#include <detail/check_args.h>
#include <detail/exception.h>
#include <avm.h>

namespace avm {
  bool CheckArgs(VMState *state, uint32_t nrecargs, uint32_t nreqargs) {
    if (nrecargs != nreqargs) {
      for (uint32_t i = 0; i < nrecargs; i++) {
        state->vm->PopStack();
      }
      state->HandleException(InvalidArgsException(nreqargs, nrecargs));
      return false;
    }
    return true;
  }
}