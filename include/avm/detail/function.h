#ifndef FUNCTION_H
#define FUNCTION_H

#include <memory>

#include <detail/variable.h>

namespace avm {
class VMState;

class Func : public Object {
public:
  Func(uint64_t, uint32_t, bool);

  void invoke(VMState *, uint32_t);
  uint64_t Address() const;
  size_t NumArgs() const;

  virtual Reference Clone(VMState *state);

  std::string ToString() const;
  std::string TypeString() const;

private:
  uint64_t addr;
  uint32_t nargs;
  bool is_variadic;
};

typedef std::shared_ptr<Func> func_ptr;
} // namespace avm

#endif