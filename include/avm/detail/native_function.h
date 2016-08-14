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
  NativeFunc(size_t nargs)
    : nargs(nargs) {
  }

  virtual void invoke(VMState *, uint32_t) = 0;
  virtual Reference clone(Heap &heap) = 0;

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

template <typename ReturnType>
class NativeFunc_NoArgs : public NativeFunc {
public:
  typedef ReturnType(*FuncType) (void);

  NativeFunc_NoArgs(FuncType ptr)
    : ptr(ptr), NativeFunc(0) {
  }

  void invoke(VMState *state, uint32_t callargs) {
    if (CheckArgs(state, nargs, callargs)) {
      ReturnType return_value = ptr();
    
      auto ref = Reference(*state->heap.AllocNull());
      auto var = new Variable();
      var->Assign(return_value);
      var->flags |= Object::FLAG_CONST;
      var->flags |= Object::FLAG_TEMPORARY;
      ref.Ref() = var;

      state->stack.push_back(ref);
    }
  }

  Reference clone(Heap &heap) {
    return Reference(*heap.AllocObject<NativeFunc_NoArgs>(ptr));
  }

  std::string TypeString() const {
    std::string result("native function");
    return result;
  }

private:
  FuncType ptr;
}; 

template <typename ReturnType, typename T1>
class NativeFunc_OneArg : public NativeFunc {
public:
  typedef ReturnType(*FuncType) (T1);

  NativeFunc_OneArg(FuncType ptr)
    : ptr(ptr), NativeFunc(1) {
  }

  void invoke(VMState *state, uint32_t callargs) {
    if (CheckArgs(state, nargs, callargs)) {
      bool error = false;
      Variable *args[1];
      for (int i = nargs - 1; i >= 0; i--) {
        auto arg = state->stack.back(); state->stack.pop_back();
        Variable *arg_casted = dynamic_cast<Variable*>(arg.Ref());
        if (!arg_casted) {
          error = true;
          state->HandleException(Exception("argument " + util::to_string(i) + " is not a valid variable"));
        } else {
          args[i] = arg_casted;
        }
      }

      if (!error) {
        ReturnType return_value = ptr(args[0]->Cast<T1>());

        auto ref = Reference(*state->heap.AllocNull());
        auto var = new Variable();
        var->Assign(return_value);
        var->flags |= Object::FLAG_CONST;
        var->flags |= Object::FLAG_TEMPORARY;
        ref.Ref() = var;

        state->stack.push_back(ref);
      }
    }
  }

  Reference clone(Heap &heap) {
    return Reference(*heap.AllocObject<NativeFunc_OneArg>(ptr));
  }

  std::string TypeString() const {
    std::string result("native function (");
    result += typeid(T1).name();
    result += ")";
    return result;
  }

private:
  FuncType ptr;
};

template <typename ReturnType, typename T1, typename T2>
class NativeFunc_TwoArgs : public NativeFunc {
public:
  typedef ReturnType(*FuncType) (T1, T2);

  NativeFunc_TwoArgs(FuncType ptr)
    : ptr(ptr), NativeFunc(2) {
  }

  void invoke(VMState *state, uint32_t callargs) {
    if (CheckArgs(state, nargs, callargs)) {
      bool error = false;
      Variable *args[2];
      for (int i = nargs - 1; i >= 0; i--) {
        auto arg = state->stack.back(); state->stack.pop_back();
        Variable *arg_casted = dynamic_cast<Variable*>(arg.Ref());
        if (!arg_casted) {
          error = true;
          state->HandleException(Exception("argument " + util::to_string(i) + " is not a valid variable"));
        } else {
          args[i] = arg_casted;
        }
      }

      if (!error) {
        ReturnType return_value = ptr(args[0]->Cast<T1>(), args[1]->Cast<T2>());

        auto ref = Reference(*state->heap.AllocNull());
        auto var = new Variable();
        var->Assign(return_value);
        var->flags |= Object::FLAG_CONST;
        var->flags |= Object::FLAG_TEMPORARY;
        ref.Ref() = var;

        state->stack.push_back(ref);
      }
    }
  }

  Reference clone(Heap &heap) {
    return Reference(*heap.AllocObject<NativeFunc_TwoArgs>(ptr));
  }

  std::string TypeString() const {
    std::string result("native function (");
    result += typeid(T1).name();
    result += ", ";
    result += typeid(T2).name();
    result += ")";
    return result;
  }

private:
  FuncType ptr;
};
} // namespace avm

#endif