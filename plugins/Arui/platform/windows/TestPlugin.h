#include <detail/vm_state.h>

#ifdef TESTPLUGIN_EXPORTS
#define TESTPLUGIN_API __declspec(dllexport)
#else
#define TESTPLUGIN_API __declspec(dllimport)
#endif

extern "C" {
  TESTPLUGIN_API void Push42(avm::VMState *state);
}