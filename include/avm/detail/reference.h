#ifndef REFERENCE_H
#define REFERENCE_H

#include <cstdint>

namespace avm {
class Object;
class Reference {
public:
    Reference();
    Reference(const Reference &other);
    explicit Reference(Object * &ref);

    inline Object **Reference::Ptr() const { return ref; }
    inline Object *&Reference::Ref() { return *ref; }
    inline const Object *const &Reference::Ref() const { return *ref; }

    void DeleteObject();

protected:
    Object **ref;
};
} // namespace avm

#endif