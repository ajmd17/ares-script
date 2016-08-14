#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <memory>

#include <detail/variable.h>

namespace avm {
class VMState;

class ArrayList : public Object {
public:
  ArrayList();

  void invoke(VMState *, uint32_t);

  virtual Reference clone(Heap &heap);

  std::string ToString() const;
  std::string TypeString() const;
};

typedef std::shared_ptr<ArrayList> arraylist_ptr;
} // namespace avm

#endif