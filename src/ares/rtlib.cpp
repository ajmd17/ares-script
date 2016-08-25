#include <rtlib.h>
#include <iostream>
#include <cstdlib>
#include <sstream>

namespace avm {
/*FILE *RuntimeLib::OpenFile(const AVMString_t &filepath, const AVMString_t &mode) {
  return fopen(filepath.c_str(), mode.c_str());
}

AVMInteger_t RuntimeLib::WriteStringToFile(FILE *stream, const AVMString_t &str) {
  return fprintf(stream, str.c_str());
}

AVMString_t RuntimeLib::ReadStringFromFile(FILE *stream, AVMInteger_t len) {
  char *str = new char[len + 1];
  fgets(str, len, stream);
  AVMString_t astr(str);
  delete[] str;
  return astr;
}

AVMInteger_t RuntimeLib::CloseFile(FILE *stream) {
  return fclose(stream);
}

AVMInteger_t RuntimeLib::ConvertToInt(AVMFloat_t f) {
  return AVMInteger_t(f);
}

bool RuntimeLib::ConvertToBool(AVMInteger_t i) {
  return !!i;
}

AVMInteger_t RuntimeLib::SystemRun(const AVMString_t &str) {
  return std::system(str.c_str());
}

AVMString_t RuntimeLib::ConsoleWrite(const AVMString_t &str) {
  std::cout << str;
  return str;
}

AVMString_t RuntimeLib::ConsoleWriteLn(const AVMString_t &str) {
  std::cout << str << "\n";
  return str;
}

AVMString_t RuntimeLib::ConsoleReadLn() {
  std::string line;
  std::getline(std::cin, line);
  return line;
}*/

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
      case Variable::type_integer:
      {
        AVMInteger_t &i = var->Cast<AVMInteger_t&>();
        result->Assign(i);
        break;
      }
      case Variable::type_float:
      {
        AVMFloat_t &f = var->Cast<AVMFloat_t&>();
        result->Assign(AVMInteger_t(f));
        break;
      }
      case Variable::type_string:
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
        case Variable::type_integer:
        {
            AVMInteger_t &i = var->Cast<AVMInteger_t&>();
            result->Assign(!!i);
            break;
        }
        case Variable::type_float:
        {
            AVMFloat_t &f = var->Cast<AVMFloat_t&>();
            result->Assign(!!f);
            break;
        }
        case Variable::type_string:
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