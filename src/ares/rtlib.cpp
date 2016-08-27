#include <rtlib.h>
#include <iostream>
#include <cstdlib>
#include <sstream>

namespace avm {
void RuntimeLib::FileIO_open(VMState *state, Object *arg1, Object *arg2) {
  bool good = true;

  Variable *filepath = dynamic_cast<Variable*>(arg1);
  if (filepath == nullptr) {
    state->HandleException(avm::TypeException(arg1->TypeString()));
    good = false;
  }

  Variable *mode = dynamic_cast<Variable*>(arg2);
  if (mode == nullptr) {
    state->HandleException(avm::TypeException(arg2->TypeString()));
    good = false;
  }

  if (good) {
    AVMString_t &filepath_str = filepath->Cast<AVMString_t&>();
    AVMString_t &mode_str = mode->Cast<AVMString_t&>();

    FILE *file = nullptr;
    fopen_s(&file, filepath_str.c_str(), mode_str.c_str());
    if (file == nullptr) {
      state->HandleException(Exception("file could not be opened"));
    }

    auto ref = Reference(*state->heap.AllocNull());
    auto result = new Variable();
    result->Assign(file);
    result->flags |= Object::FLAG_CONST;
    result->flags |= Object::FLAG_TEMPORARY;
    ref.Ref() = result;
    state->stack.push_back(ref);
  }
}

void RuntimeLib::FileIO_write(VMState *state, Object *arg1, Object *arg2) {
  bool good = true;

  Variable *stream = dynamic_cast<Variable*>(arg1);
  if (stream == nullptr) {
    state->HandleException(avm::TypeException(arg1->TypeString()));
    good = false;
  }

  Variable *content = dynamic_cast<Variable*>(arg2);
  if (content == nullptr) {
    state->HandleException(avm::TypeException(arg2->TypeString()));
    good = false;
  }

  if (good) {
    FILE *file = stream->Cast<FILE*>();
    int res = fprintf(file, content->ToString().c_str());

    auto ref = Reference(*state->heap.AllocNull());
    auto result = new Variable();
    result->Assign(res);
    result->flags |= Object::FLAG_CONST;
    result->flags |= Object::FLAG_TEMPORARY;
    ref.Ref() = result;
    state->stack.push_back(ref);
  }
}

void RuntimeLib::FileIO_read(VMState *state, Object *arg1, Object *arg2) {
  bool good = true;

  Variable *stream = dynamic_cast<Variable*>(arg1);
  if (stream == nullptr) {
    state->HandleException(avm::TypeException(arg1->TypeString()));
    good = false;
  }

  Variable *size = dynamic_cast<Variable*>(arg2);
  if (size == nullptr) {
    state->HandleException(avm::TypeException(arg2->TypeString()));
    good = false;
  }

  if (good) {
    FILE *file = stream->Cast<FILE*>();
    AVMInteger_t &len = size->Cast<AVMInteger_t&>();

    char *str = new char[len + 1];
    if (fgets(str, len, file) == nullptr) {
      state->HandleException(Exception("file could not be read from"));
    }

    AVMString_t res(str);
    delete[] str;

    auto ref = Reference(*state->heap.AllocNull());
    auto result = new Variable();
    result->Assign(res);
    result->flags |= Object::FLAG_CONST;
    result->flags |= Object::FLAG_TEMPORARY;
    ref.Ref() = result;
    state->stack.push_back(ref);
  }
}

void RuntimeLib::FileIO_close(VMState *state, Object *arg1) {
  bool good = true;

  Variable *stream = dynamic_cast<Variable*>(arg1);
  if (stream == nullptr) {
    state->HandleException(avm::TypeException(arg1->TypeString()));
    good = false;
  }

  if (good) {
    int res = fclose(stream->Cast<FILE*>());
    if (res != 0) {
      state->HandleException(Exception("file could not be closed"));
    }

    auto ref = Reference(*state->heap.AllocNull());
    auto result = new Variable();
    result->Assign(res);
    result->flags |= Object::FLAG_CONST;
    result->flags |= Object::FLAG_TEMPORARY;
    ref.Ref() = result;
    state->stack.push_back(ref);
  }
}

void RuntimeLib::Console_println(VMState *state, Object *arg1) {
  std::puts(arg1->ToString().c_str());
  auto ref = Reference(*state->heap.AllocObject<Variable>());
  ref.Ref()->flags |= Object::FLAG_TEMPORARY;
  state->stack.push_back(ref);
}

void RuntimeLib::Console_readln(VMState *state) {
  std::string res;
  std::getline(std::cin, res);

  auto ref = Reference(*state->heap.AllocNull());
  auto result = new Variable();
  result->Assign(res);
  result->flags |= Object::FLAG_CONST;
  result->flags |= Object::FLAG_TEMPORARY;
  ref.Ref() = result;
  state->stack.push_back(ref);
}

void RuntimeLib::Reflection_typeof(VMState *state, Object *arg1) {
  auto ref = Reference(*state->heap.AllocNull());
  auto result = new Variable();
  result->Assign(arg1->TypeString());
  result->flags |= Object::FLAG_CONST;
  result->flags |= Object::FLAG_TEMPORARY;
  ref.Ref() = result;
  state->stack.push_back(ref);
}

void RuntimeLib::Convert_toString(VMState *state, Object *arg1) {
  auto ref = Reference(*state->heap.AllocNull());
  auto result = new Variable();
  result->Assign(arg1->ToString());
  result->flags |= Object::FLAG_CONST;
  result->flags |= Object::FLAG_TEMPORARY;
  ref.Ref() = result;
  state->stack.push_back(ref);
}

void RuntimeLib::Convert_toInt(VMState *state, Object *arg1) {
  Variable *var = dynamic_cast<Variable*>(arg1);
  if (var == nullptr) {
    state->HandleException(avm::TypeException(arg1->TypeString()));
  } else {
    auto ref = Reference(*state->heap.AllocNull());
    auto result = new Variable();

    switch (var->type) {
    case Variable::Type_int:
    {
      AVMInteger_t &i = var->Cast<AVMInteger_t&>();
      result->Assign(i);
      break;
    }
    case Variable::Type_float:
    {
      AVMFloat_t &f = var->Cast<AVMFloat_t&>();
      result->Assign(AVMInteger_t(f));
      break;
    }
    case Variable::Type_string:
    {
      AVMString_t &s = var->Cast<AVMString_t&>();
      AVMInteger_t result_value = 0;

      if (s.length() >= 2 && (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))) {
        // parse hex
        result_value = std::stol(s, 0, 16);
        result->Assign(result_value);
      } else {
        // parse decimal
        std::stringstream ss;
        ss << s;
        if (ss >> result_value) {
          result->Assign(result_value);
        } else {
          state->HandleException(Exception("string '" + s + "' could not be parsed to int"));
        }
      }
      break;
    }
    default:
      state->HandleException(ConversionException(var->TypeString(), "int"));
      break;
    }

    result->flags |= Object::FLAG_CONST;
    result->flags |= Object::FLAG_TEMPORARY;
    ref.Ref() = result;
    state->stack.push_back(ref);
  }
}

void RuntimeLib::Convert_toBool(VMState *state, Object *arg1) {
  Variable *var = dynamic_cast<Variable*>(arg1);
  if (var == nullptr) {
    state->HandleException(avm::TypeException(arg1->TypeString()));
  } else {
    auto ref = Reference(*state->heap.AllocNull());
    auto result = new Variable();

    switch (var->type) {
    case Variable::Type_int:
    {
      AVMInteger_t &i = var->Cast<AVMInteger_t&>();
      result->Assign(!!i);
      break;
    }
    case Variable::Type_float:
    {
      AVMFloat_t &f = var->Cast<AVMFloat_t&>();
      result->Assign(!!f);
      break;
    }
    case Variable::Type_string:
    {
      AVMString_t &s = var->Cast<AVMString_t&>();
      std::transform(s.begin(), s.end(), s.begin(), ::tolower);
      std::istringstream is(s);
      bool b;
      if ((is >> std::boolalpha >> b)) {
        result->Assign(AVMInteger_t(b));
      } else {
        state->HandleException(Exception("string '" + s + "' could not be parsed to bool"));
      }
      break;
    }
    default:
      state->HandleException(ConversionException(var->TypeString(), "bool"));
      break;
    }

    result->flags |= Object::FLAG_CONST;
    result->flags |= Object::FLAG_TEMPORARY;
    ref.Ref() = result;
    state->stack.push_back(ref);
  }
}
} // namespace avm