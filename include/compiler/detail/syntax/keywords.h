#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <string>
#include <map>
#include <algorithm>
#include <utility>
#include <exception>

namespace avm {
enum Keyword {
  Keyword_invalid = -1,
  Keyword_if,
  Keyword_else,
  Keyword_do,
  Keyword_while,
  Keyword_for,
  Keyword_foreach,
  Keyword_switch,
  Keyword_case,
  Keyword_break,
  Keyword_continue,
  Keyword_default,
  Keyword_return,
  Keyword_goto,
  Keyword_try,
  Keyword_catch,
  Keyword_throw,
  Keyword_class,
  Keyword_enum,
  Keyword_object,
  Keyword_void,
  Keyword_true,
  Keyword_false,
  Keyword_null,
  Keyword_var,
  Keyword_alias,
  Keyword_using,
  Keyword_attribute,
  Keyword_static,
  Keyword_public,
  Keyword_private,
  Keyword_protected,
  Keyword_const,
  Keyword_func,
  Keyword_super,
  Keyword_module,
  Keyword_range,
  Keyword_import,
  Keyword_in,
  Keyword_is,
  Keyword_as,
  Keyword_cast,
  Keyword_print,
  Keyword_self,
  Keyword_new,
  Keyword_delete
};

const std::map<std::string, Keyword> keywords = {
  { "if", Keyword_if },
  { "else", Keyword_else },
  { "do", Keyword_do },
  { "while", Keyword_while },
  { "for", Keyword_for },
  { "foreach", Keyword_foreach },
  { "switch", Keyword_switch },
  { "case", Keyword_case },
  { "break", Keyword_break },
  { "continue", Keyword_continue },
  { "default", Keyword_default },
  { "return", Keyword_return },
  { "goto", Keyword_goto },
  { "try", Keyword_try },
  { "catch", Keyword_catch },
  { "throw", Keyword_throw },
  { "class", Keyword_class },
  { "enum", Keyword_enum },
  { "object", Keyword_object },
  { "void", Keyword_void },
  { "true", Keyword_true },
  { "false", Keyword_false },
  { "null", Keyword_null },
  { "var", Keyword_var },
  { "alias", Keyword_alias },
  { "using", Keyword_using },
  { "attribute", Keyword_attribute },
  { "static", Keyword_static },
  { "public", Keyword_public },
  { "private", Keyword_private },
  { "protected", Keyword_protected },
  { "const", Keyword_const },
  { "func", Keyword_func },
  { "super", Keyword_super },
  { "module", Keyword_module },
  { "range", Keyword_range },
  { "import", Keyword_import },
  { "in", Keyword_in },
  { "is", Keyword_is },
  { "as", Keyword_as },
  { "cast", Keyword_cast },
  { "print", Keyword_print },
  { "self", Keyword_self },
  { "new", Keyword_new },
  { "delete", Keyword_delete }
};

static std::string Keyword_ToString(Keyword kw) {
  auto it = std::find_if(keywords.begin(), keywords.end(),
    [&kw](const std::pair<std::string, Keyword> &item) {
    return item.second == kw;
  });

  if (it == keywords.end()) {
    throw std::out_of_range("Keyword not found");
  }

  return it->first;
}

static Keyword Keyword_FromString(const std::string &str) {
  auto it = keywords.find(str);
  if (it == keywords.end()) {
    return Keyword::Keyword_invalid;
  } else {
    return it->second;
  }
}
} // namespace avm

#endif