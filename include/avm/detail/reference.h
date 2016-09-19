#ifndef REFERENCE_H
#define REFERENCE_H

#include <cstdint>

namespace avm {
class Object;
class Reference {
public:
  Reference();
  explicit Reference(Object * &ref);
  Reference(const Reference &other);

  // returns the actual pointer held inside.
  Object **Ptr() const;

  Object *&Ref();
  const Object *const &Ref() const;

  void DeleteObject();

protected:
  Object **ref;
};
} // namespace avm

#endif