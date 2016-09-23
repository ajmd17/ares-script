#include <detail/reference.h>
#include <detail/object.h>

namespace avm {
Reference::Reference()
    : ref(nullptr)
{
}

Reference::Reference(const Reference &other)
    : ref(other.ref)
{
}

Reference::Reference(Object *&ref)
    : ref(&ref)
{
}

void Reference::DeleteObject()
{
    if (ref && (*ref)) {
        auto *&objptr = *ref;
        delete objptr;
        objptr = nullptr;
    }
}
} // namespace avm