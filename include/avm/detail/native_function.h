#ifndef NATIVE_FUNCTION_H
#define NATIVE_FUNCTION_H

#include <detail/exception.h>
#include <detail/variable.h>
#include <detail/vm_state.h>
#include <detail/check_args.h>
#include <common/util/to_string.h>

#include <memory>

namespace avm {
class NativeFunc : public Object {
public:
  typedef void(*FuncType) (VMState*, Object**, uint32_t);

  NativeFunc(FuncType ptr)
    : ptr(ptr) {
  }

  void invoke(VMState *state, uint32_t callargs) {
    Object **args = new Object*[callargs];

    for (int i = callargs - 1; i >= 0; i--) {
      Reference ref = state->stack.back(); state->stack.pop_back();
      args[i] = ref.Ref();
    }

    ptr(state, args, callargs);
    delete[] args;
  }

  Reference Clone(VMState *state) {
    return Reference(*state->heap.AllocObject<NativeFunc>(ptr));
  }

  std::string ToString() const {
    return "<" + TypeString() + ">";
  }

  std::string TypeString() const {
    std::string result("native function");
    return result;
  }

private:
  FuncType ptr;
};

/*class NativeFunc : public Object {
public:
  NativeFunc(size_t nargs)
    : nargs(nargs) {
  }

  virtual void invoke(VMState *, uint32_t) = 0;
  virtual Reference Clone(VMState *state) = 0;

  std::string ToString() const {
    return "<" + TypeString() + ">";
  }

  virtual std::string TypeString() const = 0;

  size_t NumArgs() const {
    return nargs;
  }

protected:
  const size_t nargs;
};

class NativeFunc_NoArgs : public NativeFunc {
public:
  typedef void(*FuncType) (VMState*);

  NativeFunc_NoArgs(FuncType ptr)
    : ptr(ptr), NativeFunc(0) {
  }

  void invoke(VMState *state, uint32_t callargs) {
    if (CheckArgs(state, nargs, callargs)) {
      ptr(state);
    }
  }

  Reference Clone(VMState *state) {
    return Reference(*state->heap.AllocObject<NativeFunc_NoArgs>(ptr));
  }

  std::string TypeString() const {
    std::string result("native function");
    return result;
  }

private:
  FuncType ptr;
};

class NativeFunc_OneArg : public NativeFunc {
public:
  typedef void(*FuncType) (VMState*, Object*);

  NativeFunc_OneArg(FuncType ptr)
    : ptr(ptr), NativeFunc(1) {
  }

  void invoke(VMState *state, uint32_t callargs) {
    if (CheckArgs(state, nargs, callargs)) {
      bool error = false;
      Object *args[1];
      for (int i = nargs - 1; i >= 0; i--) {
        Reference ref = state->stack.back(); state->stack.pop_back();
        args[i] = ref.Ref();
      }
      ptr(state, args[0]);
    }
  }

  Reference Clone(VMState *state) {
    return Reference(*state->heap.AllocObject<NativeFunc_OneArg>(ptr));
  }

  std::string TypeString() const {
    std::string result("native function");
    return result;
  }

private:
  FuncType ptr;
};

class NativeFunc_TwoArgs : public NativeFunc {
public:
  typedef void(*FuncType) (VMState*, Object*, Object*);

  NativeFunc_TwoArgs(FuncType ptr)
    : ptr(ptr), NativeFunc(2) {
  }

  void invoke(VMState *state, uint32_t callargs) {
    if (CheckArgs(state, nargs, callargs)) {
      bool error = false;
      Object *args[2];
      for (int i = nargs - 1; i >= 0; i--) {
        Reference ref = state->stack.back(); state->stack.pop_back();
        args[i] = ref.Ref();
      }
      ptr(state, args[0], args[1]);
    }
  }

  Reference Clone(VMState *state) {
    return Reference(*state->heap.AllocObject<NativeFunc_TwoArgs>(ptr));
  }

  std::string TypeString() const {
    std::string result("native function");
    return result;
  }

private:
  FuncType ptr;
};*/
} // namespace avm

#endif