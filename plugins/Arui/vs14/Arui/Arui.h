#include "widget.h"

#include <avm/detail/vm_state.h>

#ifdef ARUI_EXPORTS
#define ARUI_API __declspec(dllexport)
#else
#define ARUI_API __declspec(dllimport)
#endif

extern "C" ARUI_API void InitArui(avm::VMState *state, avm::Object **args, uint32_t argc);
extern "C" ARUI_API void DestroyArui(avm::VMState *state, avm::Object **args, uint32_t argc);
extern "C" ARUI_API void AddButton(avm::VMState *state, avm::Object **args, uint32_t argc);