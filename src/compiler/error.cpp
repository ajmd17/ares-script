#include <detail/error.h>

#include <iostream>

namespace avm {
const std::map<ErrorType, std::string> BuildMessage::messages = {
  /* Fatal errors */
  { msg_internal_error, "internal error" },
  { msg_illegal_syntax, "illegal syntax" },
  { msg_illegal_expression, "illegal expression" },
  { msg_illegal_operator, "illegal usage of operator '%'" },
  { msg_const_identifier, "'%' is const and cannot be modified" },
  { msg_prohibited_action_attribute, "attribute '%' prohibits this action" },
  { msg_unbalanced_expression, "unbalanced expression" },
  { msg_unexpected_character, "unexpected character '%'" },
  { msg_unexpected_identifier, "unexpected identifier '%'" },
  { msg_unexpected_token, "unexpected token '%'" },
  { msg_unexpected_eof, "unexpected end of file" },
  { msg_unrecognized_escape_sequence, "unrecognized escape sequence '%'" },
  { msg_unterminated_string_literal, "unterminated string literal" },
  { msg_argument_after_varargs, "argument not allowed after '...'" },
  { msg_too_many_args, "too many arguments used for function '%'" },
  { msg_too_few_args, "too few arguments used for function '%'" },
  { msg_redeclared_identifier, "identifier '%' has already been declared in this scope" },
  { msg_undeclared_identifier, "identifier '%' has not been declared" },
  { msg_expected_identifier, "expected an identifier" },
  { msg_ambiguous_identifier, "identifier '%' is ambiguous" },
  { msg_invalid_constructor, "invalid constructor" },
  { msg_unknown_class_type, "unknown class type, '%'" },
  { msg_expected_token, "expected '%'" },
  { msg_expected_module, "expected 'module' declaration" },
  { msg_module_already_defined, "module '%' has already been defined" },
  { msg_module_not_imported, "module '%' was not imported"},
  { msg_identifier_is_module, "identifier '%' is the name of a module" },
  { msg_import_not_found, "module '%' could not be found at the path '%'" },
  { msg_import_outside_global, "import not allowed outside of global scope" },
  { msg_import_current_file, "attempt to import current file" },
  { msg_self_outside_class, "'self' not allowed outside of a class" },
  { msg_else_outside_if, "'else' not connected to an if statement" },
  { msg_alias_missing_assignment, "alias '%' must have an assignment" },
  { msg_alias_must_be_identifier, "alias '%' must reference an identifier" },
  { msg_unsupported_feature, "unsupported feature" },

  /* Warnings */
  { msg_unreachable_code, "unreachable code detected" },
  { msg_expected_semicolon, "missing semicolon" },

  /* Info */
  { msg_missing_final_return, "'%' is missing a final return statement, assuming null" },
  { msg_unused_identifier, "'%' is not used" },
  { msg_empty_function_body, "the function body of '%' is empty" },
  { msg_empty_statement_body, "loop or statement body is empty" },
  { msg_module_name_begins_lowercase, "module name '%' should begin with an uppercase character"},
  { msg_function_name_begins_uppercase, "function name '%' should begin with a lowercase character"},
  { msg_variable_name_begins_uppercase, "variable name '%' should begin with a lowercase character"},
};

void BuildMessage::Display() {
  std::cout << "  " <<
    "ln: " <<
    (location.line + 1) <<
    ", col: " <<
    (location.column + 1) <<
    ": " << text << "\n";
}

bool BuildMessage::operator<(const BuildMessage &other) const {
  //if (type == other.type) {
    if (location.line == other.location.line) {
      return location.column < other.location.column;
    } else {
      return location.line < other.location.line;
    }
  //} else {
    //return type < other.type;
  //}
}
} // namespace avm