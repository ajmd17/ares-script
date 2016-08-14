#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <string>
#include <map>
#include <algorithm>
#include <utility>
#include <exception>

namespace avm {
enum st_keyword {
  kw_invalid = -1,
  kw_if,
  kw_else,
  kw_do,
  kw_while,
  kw_for,
  kw_foreach,
  kw_switch,
  kw_case,
  kw_break,
  kw_continue,
  kw_default,
  kw_return,
  kw_goto,
  kw_try,
  kw_catch,
  kw_throw,
  kw_class,
  kw_enum,
  kw_struct,
  kw_void,
  kw_true,
  kw_false,
  kw_null,
  kw_var,
  kw_alias,
  kw_using,
  kw_attribute,
  kw_static,
  kw_public,
  kw_private,
  kw_protect,
  kw_const,
  kw_func,
  kw_super,
  kw_module,
  kw_package,
  kw_import,
  kw_typeof,
  kw_is,
  kw_as,
  kw_cast,
  kw_print,
  kw_self,
  kw_new,
  kw_delete
};

const std::map<std::string, st_keyword> keywords = {
  { "if", kw_if },
  { "else", kw_else },
  { "do", kw_do },
  { "while", kw_while },
  { "for", kw_for },
  { "foreach", kw_foreach },
  { "switch", kw_switch },
  { "case", kw_case },
  { "break", kw_break },
  { "continue", kw_continue },
  { "default", kw_default },
  { "return", kw_return },
  { "goto", kw_goto },
  { "try", kw_try },
  { "catch", kw_catch },
  { "throw", kw_throw },
  { "class", kw_class },
  { "enum", kw_enum },
  { "struct", kw_struct },
  { "void", kw_void },
  { "true", kw_true },
  { "false", kw_false },
  { "null", kw_null },
  { "var", kw_var },
  { "alias", kw_alias },
  { "using", kw_using },
  { "attribute", kw_attribute },
  { "static", kw_static },
  { "pub", kw_public },
  { "priv", kw_private },
  { "protect", kw_protect },
  { "const", kw_const },
  { "func", kw_func },
  { "super", kw_super },
  { "module", kw_module },
  { "package", kw_package },
  { "import", kw_import },
  { "typeof", kw_typeof },
  { "is", kw_is },
  { "as", kw_as },
  { "cast", kw_cast },
  { "print", kw_print },
  { "self", kw_self },
  { "new", kw_new },
  { "delete", kw_delete }
};

inline std::string keyword_tostr(st_keyword kw) {
  auto it = std::find_if(keywords.begin(), keywords.end(),
    [&kw](const std::pair<std::string, st_keyword> &item) {
    return item.second == kw;
  });

  if (it == keywords.end()) {
    throw std::out_of_range("Keyword not found");
  }

  return it->first;
}

inline st_keyword keyword_fromstr(const std::string &str) {
  auto it = keywords.find(str);
  if (it == keywords.end()) {
    return st_keyword::kw_invalid;
  } else {
    return it->second;
  }
}
} // namespace avm

#endif