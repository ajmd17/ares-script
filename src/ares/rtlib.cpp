#include <rtlib.h>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <detail/native_function.h>

namespace ares {
LibLoader *RuntimeLib::libloader = nullptr;

/*template <typename T>
bool ConvertData(VMState *state, Variable *var, T &out) {
  std::stringstream ss;
  try {
    switch (var->type) {
    case Variable::Type_int:
      ss << (T)var->Cast<AVMInteger_t&>();
      break;
    case Variable::Type_float:
      ss << (T)var->Cast<AVMFloat_t&>();
      break;
    case Variable::Type_string:
    {
      AVMString_t &str = var->Cast<AVMString_t&>();
      for (char ch : str) {
        ss << (T)ch;
      }
      break;
    }
    }
  } catch (...) {
    state->HandleException(ConversionException(var->TypeString(), typeid(T).name()));
    return false;
  }

  if (!(ss >> out)) {
    state->HandleException(ConversionException(var->TypeString(), typeid(T).name()));
  }
  return false;
}*/

void RuntimeLib::FileIO_open(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 2, argc)) {
    bool good = true;

    Variable *filepath = dynamic_cast<Variable*>(args[0]);
    if (filepath == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
      good = false;
    }

    Variable *mode = dynamic_cast<Variable*>(args[1]);
    if (mode == nullptr) {
      state->HandleException(avm::TypeException(args[1]->TypeString()));
      good = false;
    }

    if (good) {
      AVMString_t &filepath_str = filepath->Cast<AVMString_t&>();
      AVMString_t &mode_str = mode->Cast<AVMString_t&>();

      FILE *file = nullptr;
#ifdef _MSC_VER
      fopen_s(&file, filepath_str.c_str(), mode_str.c_str());
#else
      file = fopen(filepath_str.c_str(), mode_str.c_str());
#endif

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
}

void RuntimeLib::FileIO_write(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 2, argc)) {
    bool good = true;

    Variable *stream = dynamic_cast<Variable*>(args[0]);
    if (stream == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
      good = false;
    }

    Variable *content = dynamic_cast<Variable*>(args[1]);
    if (content == nullptr) {
      state->HandleException(avm::TypeException(args[1]->TypeString()));
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
}

void RuntimeLib::FileIO_read(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 2, argc)) {
    bool good = true;

    Variable *stream = dynamic_cast<Variable*>(args[0]);
    if (stream == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
      good = false;
    }

    Variable *size = dynamic_cast<Variable*>(args[1]);
    if (size == nullptr) {
      state->HandleException(avm::TypeException(args[1]->TypeString()));
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
}

void RuntimeLib::FileIO_close(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 1, argc)) {
    bool good = true;

    Variable *stream = dynamic_cast<Variable*>(args[0]);
    if (stream == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
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
}

void RuntimeLib::Runtime_loadlib(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 1, argc)) {
    if (libloader == nullptr) {
      state->HandleException(Exception("library loader could not be initialized"));
      return;
    }

    Variable *var = dynamic_cast<Variable*>(args[0]);
    if (var == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
    } else {
      if (var->type == Variable::Type_string) {
        AVMString_t &s = var->Cast<AVMString_t&>();
        std::wstring ws(s.begin(), s.end());

        void *handle = RuntimeLib::libloader->LoadLib(ws);

        if (handle == nullptr) {
          state->HandleException(avm::LibraryLoadException(s));
        } else {
          auto ref = Reference(*state->heap.AllocNull());
          auto result = new Variable();
          result->Assign(handle);
          result->flags |= Object::FLAG_CONST;
          result->flags |= Object::FLAG_TEMPORARY;
          ref.Ref() = result;
          state->stack.push_back(ref);
        }
      } else {
        state->HandleException(ConversionException(var->TypeString(), "string"));
      }
    }
  }
}

void RuntimeLib::Runtime_loadfunc(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 2, argc)) {
    if (libloader == nullptr) {
      state->HandleException(Exception("library loader could not be initialized"));
      return;
    }

    // arg1 should be handle, arg2 should be function name
    Variable *var_arg1 = dynamic_cast<Variable*>(args[0]);
    if (var_arg1 == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
      return;
    }

    Variable *var_arg2 = dynamic_cast<Variable*>(args[1]);
    if (var_arg2 == nullptr) {
      state->HandleException(avm::TypeException(args[1]->TypeString()));
      return;
    }

    void *handle = nullptr;
    AVMString_t func_name;
    bool error = false;

    if (var_arg1->type == Variable::Type_native) {
      handle = var_arg1->Cast<void*>();
    } else {
      state->HandleException(ConversionException(var_arg1->TypeString(), "handle"));
      error = true;
    }

    if (var_arg2->type == Variable::Type_string) {
      func_name = var_arg2->Cast<AVMString_t>();
    } else {
      state->HandleException(ConversionException(var_arg2->TypeString(), "string"));
      error = true;
    }

    if (!error) {
      void *func_handle = libloader->LoadFunction(handle, func_name);

      if (func_handle == nullptr) {
        state->HandleException(avm::LibraryFunctionLoadException(func_name));
      } else {
        auto ext_func = (typename NativeFunc::FuncType)func_handle;

        auto ref = Reference(*state->heap.AllocNull());
        auto result = new NativeFunc(ext_func);
        result->flags |= Object::FLAG_CONST;
        result->flags |= Object::FLAG_TEMPORARY;
        ref.Ref() = result;
        state->stack.push_back(ref);
      }
    }
  }
}

void RuntimeLib::Runtime_invoke(VMState *state, Object **args, uint32_t argc) {
  if (argc == 0) {
    state->HandleException(avm::Exception("invoke() requires at least 1 parameter"));
  } else {
    auto *callee = args[0];

    // push arguments to stack
    for (uint32_t i = 1; i < argc; i++) {
      state->stack.push_back(Reference(args[i]));
    }

    callee->invoke(state, argc - 1);
    // leave pushed value on stack here
  }
}

void RuntimeLib::Console_println(VMState *state, Object **args, uint32_t argc) {
  //if (CheckArgs(state, 1, argc)) {  /* No need to check args, variadic */

  for (uint32_t i = 0; i < argc; i++) {
    std::cout << args[i]->ToString();
  }
  std::cout << "\n";

  auto ref = Reference(*state->heap.AllocObject<Variable>());
  ref.Ref()->flags |= Object::FLAG_TEMPORARY;
  state->stack.push_back(ref);
  //}
}

void RuntimeLib::Console_readln(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 0, argc)) {
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
}

/*void RuntimeLib::Console_printf(VMState *state, Object **args, uint32_t argc) {
  if (argc == 0) {
    state->HandleException(avm::Exception("printf() requires at least 1 parameter"));
  } else {
    auto *fmt = args[0];

    // arg1 should be handle, arg2 should be function name
    Variable *var_fmt = dynamic_cast<Variable*>(args[0]);
    if (var_fmt == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
      return;
    }

    bool good = true;
    AVMString_t fmt_str;

    if (var_fmt->type == Variable::Type_string) {
      fmt_str = var_fmt->Cast<AVMString_t>();
    } else {
      state->HandleException(ConversionException(var_fmt->TypeString(), "string"));
      good = false;
    }

    if (good) {

      size_t arg = 1;
      for (size_t i = 0; i < fmt_str.length(); i++) {
        if (arg < argc && fmt_str[i] == '%') {

          std::string str = args[arg]->ToString();

          // read next char
          if (fmt_str[i + 1] == 's') {
             std::cout << str.c_str();
            ++i;
          } else if (fmt_str[i + 1] == 'd') {
            std::cout << std::stoll(str, nullptr, 10);
            ++i;
          } else if (fmt_str[i + 1] == 'x') {
            std::cout << std::hex << std::stoll(str, nullptr, 10);
            ++i;
          } else if (fmt_str[i + 1] == 'X') {
            std::cout << std::hex << std::uppercase << std::stoll(str, nullptr, 10);
            ++i;
          } else if (fmt_str[i + 1] == 'f') {
            std::cout << std::stof(str);
            ++i;
          } else if (isdigit(fmt_str[i + 1])) {
            // read until no longer digit
            std::string width;
            while (isdigit(fmt_str[++i])) {
              width += fmt_str[i];
            }

            if (fmt_str[i] == '.') {

              std::string precision;
              while (isdigit(fmt_str[++i])) {
                precision += fmt_str[i];
              }

              std::cout << std::setw(std::stoi(width));
              std::setprecision(std::stoi(precision));
              std::cout << std::stof(str);
            } else {
              std::cout << std::setw(std::stoi(width));
              std::cout << std::stof(str);
            }
            ++i;
          }
          ++arg;
        } else {
          std::cout << fmt_str[i];
        }
      }

      auto ref = Reference(*state->heap.AllocNull());
      auto result = new Variable();
      result->Assign(fmt_str.length());
      result->flags |= Object::FLAG_CONST;
      result->flags |= Object::FLAG_TEMPORARY;
      ref.Ref() = result;
      state->stack.push_back(ref);
    }
  }
}*/

void RuntimeLib::Console_system(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 1, argc)) {
    Variable *var = dynamic_cast<Variable*>(args[0]);
    if (var == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
    } else {
      if (var->type == Variable::Type_string) {
        AVMString_t &s = var->Cast<AVMString_t&>();
        int res = std::system(s.c_str());
        auto ref = Reference(*state->heap.AllocNull());
        auto result = new Variable();
        result->Assign(res);
        result->flags |= Object::FLAG_CONST;
        result->flags |= Object::FLAG_TEMPORARY;
        ref.Ref() = result;
        state->stack.push_back(ref);
      } else {
        state->HandleException(ConversionException(var->TypeString(), "string"));
      }
    }
  }
}

void RuntimeLib::Reflection_typeof(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 1, argc)) {
    auto ref = Reference(*state->heap.AllocNull());
    auto result = new Variable();
    result->Assign(args[0]->TypeString());
    result->flags |= Object::FLAG_CONST;
    result->flags |= Object::FLAG_TEMPORARY;
    ref.Ref() = result;
    state->stack.push_back(ref);
  }
}

void RuntimeLib::Convert_toString(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 1, argc)) {
    auto ref = Reference(*state->heap.AllocNull());
    auto result = new Variable();
    result->Assign(args[0]->ToString());
    result->flags |= Object::FLAG_CONST;
    result->flags |= Object::FLAG_TEMPORARY;
    ref.Ref() = result;
    state->stack.push_back(ref);
  }
}

void RuntimeLib::Convert_toInt(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 1, argc)) {
    Variable *var = dynamic_cast<Variable*>(args[0]);
    if (var == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
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
          result_value = std::strtol(s.c_str(), 0, 16);
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
}

void RuntimeLib::Convert_toFloat(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 1, argc)) {
    Variable *var = dynamic_cast<Variable*>(args[0]);
    if (var == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
    } else {
      auto ref = Reference(*state->heap.AllocNull());
      auto result = new Variable();

      switch (var->type) {
      case Variable::Type_int:
      {
        AVMInteger_t &i = var->Cast<AVMInteger_t&>();
        result->Assign((AVMFloat_t)i);
        break;
      }
      case Variable::Type_float:
      {
        AVMFloat_t &f = var->Cast<AVMFloat_t&>();
        result->Assign(f);
        break;
      }
      case Variable::Type_string:
      {
        AVMString_t &s = var->Cast<AVMString_t&>();
        std::istringstream is(s);
        AVMFloat_t f;
        if ((is >> f)) {
          result->Assign(f);
        } else {
          state->HandleException(Exception("string '" + s + "' could not be parsed to float"));
        }
        break;
      }
      default:
        state->HandleException(ConversionException(var->TypeString(), "float"));
        break;
      }

      result->flags |= Object::FLAG_CONST;
      result->flags |= Object::FLAG_TEMPORARY;
      ref.Ref() = result;
      state->stack.push_back(ref);
    }
  }
}

void RuntimeLib::Convert_toBool(VMState *state, Object **args, uint32_t argc) {
  if (CheckArgs(state, 1, argc)) {
    Variable *var = dynamic_cast<Variable*>(args[0]);
    if (var == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
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
}
} // namespace avm