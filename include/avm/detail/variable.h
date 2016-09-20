#ifndef VARIABLE_H
#define VARIABLE_H

#include <detail/dynamic.h>
#include <detail/object.h>
#include <detail/heap.h>
#include <detail/StackValue.h>

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
        Type_none,
        Type_int,
        Type_float,
        Type_string,
        Type_struct,
        Type_native,
    } type;

    /** Sets the inner value to null by default */
    Variable();

    /** Sets the inner value */
    template <typename T>
    explicit Variable(T value)
    {
        typedef typename std::decay<T>::type Decayed;
        SetValue<Decayed>(value);
    }

    ~Variable();

    /** Assign a new value */
    template <typename T>
    void Assign(T value)
    {
        typedef typename std::decay<T>::type Decayed;
        SetValue<Decayed>(value);
    }

    virtual void invoke(VMState *, uint32_t);
    virtual Reference Clone(VMState *state);

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
    T Cast()
    {
        typedef typename std::decay<T>::type Decayed;
        if (this == nullptr) {
            std::cout << "cast() used on null value\n";
            throw "null value";
        }
        return GetValue<Decayed, T>();
    }

protected:
    /** Holds a heap value */
    Dynamic value;
    /** Numeric types are stored on the stack */
    StackValue stack_value;

private:
    template <typename Decayed, typename T>
    typename std::enable_if<std::is_arithmetic<Decayed>::value, T>::type
        GetValue()
    {
        if (type == Type_int) {
            return (T)stack_value.int_value;
        } else if (type == Type_float) {
            return (T)stack_value.float_value;
        } else {
            throw "No conversion";
        }
    }

    template <typename Decayed, typename T>
    typename std::enable_if<!std::is_arithmetic<Decayed>::value, T>::type
        GetValue()
    {
        return value.Get<T>();
    }

    /** Converts types such as int and long to the internal representation, avm_int */
    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type
        SetValue(T t)
    {
        type = Type_int;
        stack_value.int_value = static_cast<AVMInteger_t>(t);
    }

    /** Converts types such as double and float to the internal representation, avm_float */
    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type
        SetValue(T t)
    {
        type = Type_float;
        stack_value.float_value = static_cast<AVMFloat_t>(t);
    }

    /** Does not do any conversion for string objects */
    template <typename T>
    typename std::enable_if<std::is_same<std::string, T>::value, void>::type
        SetValue(T t)
    {
        type = Type_string;
        value.Assign(t);
    }

    /** Native object (must also specify that it is not a std::string) */
    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value || (std::is_class<T>::value && !std::is_same<std::string, T>::value),
        void>::type
        SetValue(T t)
    {
        type = Type_native;
        value.Assign(t);
    }
};
} // namespace avm

#endif