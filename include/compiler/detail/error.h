#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <sstream>
#include <map>

#include <detail/location.h>

namespace avm {
enum ErrorLevel {
  err_info,
  err_warning,
  err_fatal
};

enum ErrorType {
  /* Fatal errors */
  msg_internal_error,
  msg_illegal_syntax,
  msg_illegal_expression,
  msg_illegal_operator,
  msg_const_identifier,
  msg_prohibited_action_attribute,
  msg_unbalanced_expression,
  msg_unexpected_character,
  msg_unexpected_identifier,
  msg_unexpected_token,
  msg_unexpected_eof,
  msg_unrecognized_escape_sequence,
  msg_unterminated_string_literal,
  msg_argument_after_varargs,
  msg_too_many_args,
  msg_too_few_args,
  msg_redeclared_identifier,
  msg_undeclared_identifier,
  msg_expected_identifier,
  msg_ambiguous_identifier,
  msg_invalid_constructor,
  msg_unknown_class_type,
  msg_expected_token,
  msg_expected_module,
  msg_module_already_defined,
  msg_module_not_imported,
  msg_import_not_found,
  msg_identifier_is_module,
  msg_import_outside_global,
  msg_import_current_file,
  msg_self_outside_class,
  msg_else_outside_if,
  msg_alias_missing_assignment,
  msg_alias_must_be_identifier,
  msg_unsupported_feature,

  /* Warnings */
  msg_unreachable_code,
  msg_expected_semicolon,

  /* Info */
  msg_missing_final_return,
  msg_unused_identifier,
  msg_empty_function_body,
  msg_empty_statement_body,
  msg_module_name_begins_lowercase,
  msg_function_name_begins_uppercase,
  msg_variable_name_begins_uppercase,
};

class BuildMessage {
public:
  template <typename ... Args>
  BuildMessage(ErrorType type, ErrorLevel level, SourceLocation location, Args && ... args)
    : type(type), level(level), location(location) {

    switch (level) {
    case err_info:
      text = "INFO: "; break;
    case err_warning:
      text = "WARNING: "; break;
    case err_fatal:
      text = "FATAL: "; break;
    }

    MakeMessage(messages.at(type).c_str(), args...);
  }

  void Display();
  bool operator<(const BuildMessage &other) const;

  ErrorLevel level;
  ErrorType type;
  SourceLocation location;

  std::string text;
  std::string filename;

private:
  void MakeMessage(const char *format) {
    text += format;
  }

  template <typename T, typename ... Args>
  void MakeMessage(const char *format, T value, Args && ... args) {
    for (; *format != '\0'; format++) {
      if (*format == '%') {
        std::ostringstream sstream;
        sstream << value;
        text += sstream.str();
        MakeMessage(format + 1, args...);
        return;
      }
      text += *format;
    }
  }

  static const std::map<ErrorType, std::string> messages;
};
} // namespace avm

#endif