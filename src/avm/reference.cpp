#include <detail/reference.h>
#include <detail/object.h>

namespace avm {

Reference::Reference() 
    : ref(nullptr) {
}

Reference::Reference(Object *&ref) 
    : ref(&ref) {
}

Reference::Reference(const Reference &other) 
    : ref(other.ref) {
}

Object **Reference::Ptr() const {
  return ref;
}

Object *&Reference::Ref() {
  return *ref;
}

const Object *const &Reference::Ref() const {
  return *ref;
}

void Reference::DeleteObject() {
  if (ref && (*ref)) {
    auto *&objptr = *ref;
    delete objptr;
    objptr = nullptr;
  }
}
} // namespace avm