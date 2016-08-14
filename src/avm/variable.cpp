#include <detail/variable.h>

#include <avm.h>
#include <detail/vm_state.h>
#include <detail/exception.h>
#include <common/util/to_string.h>

namespace avm {
Variable::Variable() {
  value.Assign(nullptr);
  type = type_null;
}

Variable::~Variable() {
}

void Variable::invoke(VMState *state, uint32_t callargs) {
  state->HandleException(BadInvokeException(TypeString()));
}

Reference Variable::clone(Heap &heap) {
  auto ref = Reference(*heap.AllocNull());

  auto var = new Variable();
  var->value = value;
  var->type = type;
  ref.Ref() = var;

  // copy all members
  for (auto &&member : fields) {
    if (member.second.Ref() != nullptr) {
      ref.Ref()->AddFieldReference(member.first, member.second.Ref()->clone(heap));
    }
  }

  return ref;
}

Variable &Variable::Add(VMState *state, Variable *other) {
  if (type == type_string) {
    auto &str1 = value.Get<AVMString_t&>();
    auto str2 = other->ToString();

    str1 = str1 + str2;
  } else if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 + v2;
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();
    auto v2 = other->Cast<AVMFloat_t>();

    v1 = v1 + v2;
  } else if (other->type == type_float) {
    auto v1 = Cast<AVMFloat_t>();
    auto &v2 = other->Cast<AVMFloat_t&>();

    SetValue(v1 + v2);
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "+" }));
  }

  return *this;
}

Variable &Variable::Subtract(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 - v2;
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();
    auto v2 = other->Cast<AVMFloat_t>();

    v1 -= v2;
  } else if (other->type == type_float) {
    auto v1 = Cast<AVMFloat_t>();
    auto &v2 = other->Cast<AVMFloat_t&>();

    SetValue(v1 - v2);
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "-" }));
  }

  return *this;
}

Variable &Variable::Multiply(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 * v2;
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();
    auto v2 = other->Cast<AVMFloat_t>();

    v1 *= v2;
  } else if (other->type == type_float) {
    auto v1 = Cast<AVMFloat_t>();
    auto &v2 = other->Cast<AVMFloat_t&>();

    SetValue(v1 * v2);
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "*" }));
  }

  return *this;
}

Variable &Variable::Power(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = std::pow(v1, v2);
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();
    auto v2 = other->Cast<AVMFloat_t>();

    v1 = std::pow(v1, v2);
  } else if (other->type == type_float) {
    auto v1 = Cast<AVMFloat_t>();
    auto &v2 = other->Cast<AVMFloat_t&>();

    SetValue(std::pow(v1, v2));
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "pow" }));
  }

  return *this;
}

Variable &Variable::Divide(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 / v2;
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();
    auto v2 = other->Cast<AVMFloat_t>();

    v1 /= v2;
  } else if (other->type == type_float) {
    auto v1 = Cast<AVMFloat_t>();
    auto &v2 = other->Cast<AVMFloat_t&>();

    SetValue(v1 / v2);
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "/" }));
  }

  return *this;
}

Variable &Variable::Modulus(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 % v2;
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "mod" }));
  }

  return *this;
}

Variable &Variable::BitwiseXor(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 ^ v2;
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "^" }));
  }

  return *this;
}

Variable &Variable::BitwiseAnd(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 & v2;
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "&" }));
  }

  return *this;
}

Variable &Variable::BitwiseOr(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 | v2;
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "|" }));
  }

  return *this;
}

Variable &Variable::LeftShift(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 << v2;
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "<<" }));
  }

  return *this;
}

Variable &Variable::RightShift(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 >> v2;
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), ">>" }));
  }

  return *this;

}

Variable &Variable::LogicalAnd(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 && v2;
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "&&" }));
  }

  return *this;
}

Variable &Variable::LogicalOr(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 || v2;
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "||" }));
  }

  return *this;
}

Variable &Variable::Equals(VMState *state, Variable *other) {
  if (other->type == type_null) {
    SetValue(AVMInteger_t(type == type_null));
  } else if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    v1 = v1 == v2;
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();
    auto v2 = other->Cast<AVMFloat_t>();

    SetValue(AVMInteger_t(v1 == v2));
  } else if (other->type == type_float) {
    auto v1 = Cast<AVMFloat_t>();
    auto &v2 = other->Cast<AVMFloat_t&>();

    SetValue(AVMInteger_t(v1 == v2));
  } else if ((type == type_string) && (other->type == type_string)) {
    auto &str1 = value.Get<AVMString_t&>();
    auto &str2 = other->value.Get<AVMString_t&>();

    SetValue(AVMInteger_t(str1 == str2));
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "==" }));
  }

  return *this;
}

Variable &Variable::NotEqual(VMState *state, Variable *other) {
  if (other->type == type_null) {
    SetValue(AVMInteger_t(type != type_null));
  } if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    SetValue(AVMInteger_t(v1 != v2));
  } else if (type == type_float) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMFloat_t>();

    SetValue(AVMInteger_t(v1 != v2));
  } else if (other->type == type_float) {
    auto v1 = Cast<AVMFloat_t>();
    auto &v2 = other->Cast<AVMFloat_t&>();

    SetValue(AVMInteger_t(v1 != v2));
  } else if ((type == type_string) && (other->type == type_string)) {
    auto &str1 = value.Get<AVMString_t&>();
    auto &str2 = other->value.Get<AVMString_t&>();

    SetValue(AVMInteger_t(str1 != str2));
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "!=" }));
  }

  return *this;
}

Variable &Variable::Less(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    SetValue(AVMInteger_t(v1 < v2));
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();
    auto v2 = other->Cast<AVMFloat_t>();

    SetValue(AVMInteger_t(v1 < v2));
  } else if (other->type == type_float) {
    auto v1 = Cast<AVMFloat_t>();
    auto &v2 = other->Cast<AVMFloat_t&>();

    SetValue(AVMInteger_t(v1 < v2));
  } else if ((type == type_string) && (other->type == type_string)) {
    auto &str1 = value.Get<AVMString_t&>();
    auto &str2 = other->value.Get<AVMString_t&>();

    SetValue(AVMInteger_t(str1 < str2));
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "<" }));
  }

  return *this;
}

Variable &Variable::Greater(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    SetValue(AVMInteger_t(v1 > v2));
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();
    auto v2 = other->Cast<AVMFloat_t>();

    SetValue(AVMInteger_t(v1 > v2));
  } else if (other->type == type_float) {
    auto v1 = Cast<AVMFloat_t>();
    auto &v2 = other->Cast<AVMFloat_t&>();
    
    SetValue(AVMInteger_t(v1 > v2));
  } else if ((type == type_string) && (other->type == type_string)) {
    auto &str1 = value.Get<AVMString_t&>();
    auto &str2 = other->value.Get<AVMString_t&>();

    SetValue(AVMInteger_t(str1 > str2));
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), ">" }));
  }

  return *this;
}

Variable &Variable::LessOrEqual(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    SetValue(AVMInteger_t(v1 <= v2));
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();
    auto v2 = other->Cast<AVMFloat_t>();

    SetValue(AVMInteger_t(v1 <= v2));
  } else if (other->type == type_float) {
    auto v1 = Cast<AVMFloat_t>();
    auto &v2 = other->Cast<AVMFloat_t&>();

    SetValue(AVMInteger_t(v1 <= v2));
  } else if ((type == type_string) && (other->type == type_string)) {
    auto &str1 = value.Get<AVMString_t&>();
    auto &str2 = other->value.Get<AVMString_t&>();

    SetValue(AVMInteger_t(str1 <= str2));
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), "<=" }));
  }

  return *this;
}

Variable &Variable::GreaterOrEqual(VMState *state, Variable *other) {
  if ((type == type_integer) && (other->type == type_integer)) {
    auto &v1 = Cast<AVMInteger_t&>();
    auto v2 = other->Cast<AVMInteger_t>();

    SetValue(AVMInteger_t(v1 >= v2));
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();
    auto v2 = other->Cast<AVMFloat_t>();

    SetValue(AVMInteger_t(v1 >= v2));
  } else if (other->type == type_float) {
    auto v1 = Cast<AVMFloat_t>();
    auto &v2 = other->Cast<AVMFloat_t&>();

    SetValue(AVMInteger_t(v1 >= v2));
  } else if ((type == type_string) && (other->type == type_string)) {
    auto &str1 = value.Get<AVMString_t&>();
    auto &str2 = other->value.Get<AVMString_t&>();

    SetValue(AVMInteger_t(str1 >= str2));
  } else {
    state->HandleException(BinOpException({ TypeString(), other->TypeString(), ">=" }));
  }

  return *this;
}

Variable &Variable::LogicalNot(VMState *state) {
  if (type == type_null) {
    SetValue(1);
  } else if (type == type_integer) {
    auto &v1 = Cast<AVMInteger_t&>();

    v1 = !v1;
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();

    v1 = !v1;
  } else {
    state->HandleException(UnOpException({ TypeString(), "!" }));
  }

  return *this;
}

Variable &Variable::Negate(VMState *state) {
  if (type == type_integer) {
    auto &v1 = Cast<AVMInteger_t&>();

    v1 = -v1;
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();

    v1 = -v1;
  } else {
    state->HandleException(UnOpException({ TypeString(), "-" }));
  }

  return *this;
}

/// \todo actually add pre and post incrementing
Variable &Variable::PreIncrement(VMState *state) {
  if (type == type_integer) {
    auto &v1 = Cast<AVMInteger_t&>();

    v1++;
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();

    v1++;
  } else {
    state->HandleException(UnOpException({ TypeString(), "-" }));
  }

  return *this;
}

Variable &Variable::PreDecrement(VMState *state) {
  if (type == type_integer) {
    auto &v1 = Cast<AVMInteger_t&>();

    v1--;
  } else if (type == type_float) {
    auto &v1 = Cast<AVMFloat_t&>();

    v1--;
  } else {
    state->HandleException(UnOpException({ TypeString(), "-" }));
  }

  return *this;
}

Variable &Variable::PostIncrement(VMState *state) {
  return PreIncrement(state);
}

Variable &Variable::PostDecrement(VMState *state) {
  return PreDecrement(state);
}

std::string Variable::ToString() const {
  switch (type) {
  case type_integer:
    return util::to_string(value.Get<AVMInteger_t>());
  case type_float:
    return util::to_string(value.Get<AVMFloat_t>());
  case type_string:
    return value.Get<AVMString_t>();
  case type_structure:
  {
    std::string result = "{";
    for (size_t i = 0; i < fields.size(); i++) {
      auto &it = fields[i];
      result += "\"" + it.first + "\"=" + it.second.Ref()->ToString();
      if (i < fields.size() - 1) {
        result += ", ";
      }
    }
    result += "}";
    return result;
  }
  default:
    return "<" + TypeString() + ">";
  }
}

std::string Variable::TypeString() const {
  switch (type) {
  case type_null:
    return "null";
  case type_integer:
    return "integer";
  case type_float:
    return "float";
  case type_string:
    return "string";
  case type_structure:
    return "structure";
  case type_native:
    return std::string("native ") + value.TypeInfo().name();
  default:
    return value.TypeInfo().name();
  }
}
} // namespace avm