#ifndef VARIABLE_H
#define VARIABLE_H

#include <detail/dynamic.h>
#include <detail/object.h>
#include <detail/heap.h>
#include <common/types.h>

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <type_traits>

namespace avm {
class VMState;

class Variable : public Object {
public:
  enum {
    type_null,
    type_integer,
    type_float,
    type_string,
    type_structure,
    type_native,
  } type;

  /** Sets the inner value to null by default */
  Variable();

  /** Sets the inner value */
  template <typename T>
  explicit Variable(T value) {
    typedef typename std::decay<T>::type Decayed;
    SetValue<Decayed>(value);
  }

  ~Variable();

  /** Assign a new value */
  template <typename T>
  void Assign(T value) {
    typedef typename std::decay<T>::type Decayed;
    SetValue<Decayed>(value);
  }

  virtual void invoke(VMState *, uint32_t);
  virtual Reference clone(Heap &heap);

  /** Arithmetic operations */
  Variable &Add(VMState *state, Variable *other);
  Variable &Subtract(VMState *state, Variable *other);
  Variable &Multiply(VMState *state, Variable *other);
  Variable &Power(VMState *state, Variable *other);
  Variable &Divide(VMState *state, Variable *other);
  Variable &Modulus(VMState *state, Variable *other);
  Variable &BitwiseXor(VMState *state, Variable *other);
  Variable &BitwiseAnd(VMState *state, Variable *other);
  Variable &BitwiseOr(VMState *state, Variable *other);
  Variable &LeftShift(VMState *state, Variable *other);
  Variable &RightShift(VMState *state, Variable *other);
  Variable &LogicalAnd(VMState *state, Variable *other);
  Variable &LogicalOr(VMState *state, Variable *other);
  Variable &Equals(VMState *state, Variable *other);
  Variable &NotEqual(VMState *state, Variable *other);
  Variable &Less(VMState *state, Variable *other);
  Variable &Greater(VMState *state, Variable *other);
  Variable &LessOrEqual(VMState *state, Variable *other);
  Variable &GreaterOrEqual(VMState *state, Variable *other);
  Variable &LogicalNot(VMState *state);
  Variable &Negate(VMState *state);
  Variable &PreIncrement(VMState *state);
  Variable &PreDecrement(VMState *state);
  Variable &PostIncrement(VMState *state);
  Variable &PostDecrement(VMState *state);

  /** Represent object as a string */
  virtual std::string ToString() const;
  /** Represent type of this object as string*/
  std::string TypeString() const;
  /** Return the actual C++ type of this object */
  const std::type_info &TypeInfo() const { return value.TypeInfo(); }

  /** This function will attempt to perform a conversion of types,
      if the requested type is different than the type of the held value.
      Otherwise, the original value is retrieved.
  */
  template <typename T>
  T Cast() const {
    typedef typename std::decay<T>::type Decayed;

    if (this == nullptr || value.IsNull()) {
      std::cout << "cast() used on null value\n";
      throw "null value";
    }

    if (typeid(Decayed) != TypeInfo()) {
      if (std::is_reference<T>::value) {
        std::cout << "cannot return a reference of a different type\n";
        throw "cannot return reference";
      } else {
        // convert inner type
        bool success = false;
        Decayed converted = Conversions::Convert<Decayed>(this, success);
        if (!success) {
          std::cout << "no conversion '" << value.TypeInfo().name() << "' -> '" << typeid(Decayed).name() << "'\n";
          throw "no conversion";
        } else {
          return converted;
        }
      }
    }

    return value.Get<T>();
  }

//protected:
  /** Holds the actual value */
  Dynamic value;

private:
  /** Converts types such as int and long to the internal representation, avm_int */
  template <typename T>
  typename std::enable_if<std::is_integral<T>::value, void>::type
  SetValue(T t) {
    type = type_integer;
    value.Assign(static_cast<AVMInteger_t>(t));
  }

  /** Converts types such as double and float to the internal representation, avm_float */
  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, void>::type
  SetValue(T t) {
    type = type_float;
    value.Assign(static_cast<AVMFloat_t>(t));
  }

  /** Does not do any conversion for string objects */
  template <typename T>
  typename std::enable_if<std::is_same<std::string, T>::value, void>::type
  SetValue(T t) {
    type = type_string;
    value.Assign(t);
  }

  /** Native object (must also specify that it is not a std::string) */
  template <typename T> 
  typename std::enable_if<std::is_pointer<T>::value || (std::is_class<T>::value && !std::is_same<std::string, T>::value), 
    void>::type 
  SetValue(T t) {
    type = type_native;
    value.Assign(t);
  }

  /** Allows a variable to be converted to a similar type of value.
      Mainly used for things like adding an integer to a float, where
      the integer would be converted to a float.
  */
  struct Conversions {
    template <typename T>
    typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value, T>::type
      static Convert(const Variable *const in_val, bool &success) {
      success = true;
      if (in_val->value.Compatible<double>()) {
        const auto v = in_val->value.Get<double>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<long double>()) {
        const auto v = in_val->value.Get<long double>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<float>()) {
        const auto v = in_val->value.Get<float>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<bool>()) {
        const auto v = in_val->value.Get<bool>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<int>()) {
        const auto v = in_val->value.Get<int>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<unsigned int>()) {
        const auto v = in_val->value.Get<unsigned int>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<short>()) {
        const auto v = in_val->value.Get<short>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<unsigned short>()) {
        const auto v = in_val->value.Get<unsigned short>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<char>()) {
        const auto v = in_val->value.Get<char>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<unsigned char>()) {
        const auto v = in_val->value.Get<unsigned char>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<long>()) {
        const auto v = in_val->value.Get<long>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<unsigned long>()) {
        const auto v = in_val->value.Get<unsigned long>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<long long>()) {
        const auto v = in_val->value.Get<long long>();
        return static_cast<T>(v);
      } else if (in_val->value.Compatible<unsigned long long>()) {
        const auto v = in_val->value.Get<unsigned long long>();
        return static_cast<T>(v);
      } else {
        success = false;
        return T();
      }
    }

    // for other objects
    template <typename T>
    typename std::enable_if<!(std::is_integral<T>::value || std::is_floating_point<T>::value), T>::type
      static Convert(const Variable *const in_val, bool &success) {
      success = false;
      throw "not implemented";
    }
  };
};
} // namespace avm

#endif