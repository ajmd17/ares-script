#ifndef AVM_EXCEPTION_H
#define AVM_EXCEPTION_H

#include <iostream>
#include <string>

#include <common/util/to_string.h>

namespace avm {
struct Exception {
  std::string message;

  Exception()
    : message("exception") {
  }

  Exception(const std::string &message)
    : message(message) {
  }
};

struct BinOpException : public Exception {
  BinOpException(const std::string &a, const std::string &b, const std::string &op)
    : Exception("invalid operator '" + op + "' between types '" + a + "', '" + b + "'") {
  }
};

struct UnOpException : public Exception {
  UnOpException(const std::string &type, const std::string &op)
    : Exception("invalid operator '" + op + "' on type '" + type + "'") {
  }
};

struct ConstException : public Exception {
  ConstException()
    : Exception("object is const") {
  }
};

struct NullRefException : public Exception {
  NullRefException()
    : Exception("object is null") {
  }
};

struct TypeException : public Exception {
  TypeException(const std::string &type)
    : Exception("invalid type: '" + type + "'") {
  }
};

struct ConversionException : public Exception {
  ConversionException(const std::string &type_from, const std::string &type_to)
    : Exception("cannot convert from '" + type_from + "' to '" + type_to + "'") {
  }
};

struct InvalidArgsException : public Exception {
  InvalidArgsException(int expected, int provided)
    : Exception("invalid number of arguments, expected " +
        util::to_string(expected) + ", got " + util::to_string(provided)) {
  }
};

struct BadInvokeException : public Exception {
  BadInvokeException(const std::string &typestr)
    : Exception("cannot invoke object, not a function type. Type: '" + typestr + "'") {
  }
};

struct MemberNotFoundException : public Exception {
  MemberNotFoundException(const std::string &name)
    : Exception("member '" + name + "' not found") {
  }
};
} // namespace avm

#endif