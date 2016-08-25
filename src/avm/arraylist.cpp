#include <detail/arraylist.h>
#include <detail/reference.h>
#include <detail/vm_state.h>

namespace avm {
Array::Array() {
}

void Array::invoke(VMState *, uint32_t) {
  throw std::runtime_error("not a function");
}

Reference Array::Clone(VMState *state) {
  auto ref = Reference(*state->heap.AllocObject<Array>());

  // copy all members
  for (auto &&member : fields) {
    ref.Ref()->AddFieldReference(state, member.first, member.second.Ref()->Clone(state));
  }

  return ref;
}

std::string Array::ToString() const {
  return TypeString();
}

std::string Array::TypeString() const {
  return "array";
}
} // namespace avm