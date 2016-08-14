#include <detail/arraylist.h>
#include <detail/reference.h>

namespace avm {
ArrayList::ArrayList() {
}

void ArrayList::invoke(VMState *, uint32_t) {
  throw std::runtime_error("not a function");
}

Reference ArrayList::clone(Heap &heap) {
  auto ref = Reference(*heap.AllocObject<ArrayList>());

  // copy all members
  for (auto &&member : fields) {
    ref.Ref()->AddFieldReference(member.first, member.second.Ref()->clone(heap));
  }

  return ref;
}

std::string ArrayList::ToString() const {
  return TypeString();
}

std::string ArrayList::TypeString() const {
  return "arraylist";
}
} // namespace avm