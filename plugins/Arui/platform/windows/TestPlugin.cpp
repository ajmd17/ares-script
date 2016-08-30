// TestPlugin.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "TestPlugin.h"

#include <detail/variable.h>

extern "C" {
  TESTPLUGIN_API void Push42(avm::VMState *state) {
    auto ref = avm::Reference(*state->heap.AllocNull());
    auto result = new avm::Variable();
    result->Assign(42);
    result->flags |= avm::Object::FLAG_CONST;
    result->flags |= avm::Object::FLAG_TEMPORARY;
    ref.Ref() = result;
    state->stack.push_back(ref);
  }
}
