#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <sstream>
#include <map>

#include <detail/location.h>

namespace avm {
enum ErrorLevel {
    Level_info,
    Level_warning,
    Level_fatal
};

enum ErrorType {
    /* Fatal errors */
    Msg_internal_error,
    Msg_illegal_syntax,
    Msg_illegal_expression,
    Msg_illegal_operator,
    Msg_const_identifier,
    Msg_prohibited_action_attribute,
    Msg_unbalanced_expression,
    Msg_unexpected_character,
    Msg_unexpected_identifier,
    Msg_unexpected_token,
    Msg_unexpected_eof,
    Msg_unrecognized_escape_sequence,
    Msg_unterminated_string_literal,
    Msg_argument_after_varargs,
    Msg_too_many_args,
    Msg_too_few_args,
    Msg_redeclared_identifier,
    Msg_undeclared_identifier,
    Msg_expected_identifier,
    Msg_ambiguous_identifier,
    Msg_invalid_constructor,
    Msg_unknown_class_type,
    Msg_expected_token,
    Msg_expected_module,
    Msg_module_already_defined,
    Msg_module_not_imported,
    Msg_import_not_found,
    Msg_identifier_is_module,
    Msg_import_outside_global,
    Msg_import_current_file,
    Msg_self_outside_class,
    Msg_else_outside_if,
    Msg_alias_missing_assignment,
    Msg_alias_must_be_identifier,
    Msg_unrecognized_alias_type,
    Msg_unsupported_feature,

    /* Warnings */
    Msg_unreachable_code,
    Msg_expected_semicolon,

    /* Info */
    //Msg_missing_final_return,
    Msg_unused_identifier,
    Msg_empty_function_body,
    Msg_empty_statement_body,
    Msg_module_name_begins_lowercase,
   // Msg_function_name_begins_uppercase,
   // Msg_variable_name_begins_uppercase,
};

class BuildMessage {
public:
    template <typename ... Args>
    BuildMessage(ErrorType type, ErrorLevel level, SourceLocation location, Args && ... args)
        : type(type), level(level), location(location)
    {
        switch (level) {
        case Level_info:
            text = "INFO: "; break;
        case Level_warning:
            text = "WARNING: "; break;
        case Level_fatal:
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
    void MakeMessage(const char *format)
    {
        text += format;
    }

    template <typename T, typename ... Args>
    void MakeMessage(const char *format, T value, Args && ... args)
    {
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