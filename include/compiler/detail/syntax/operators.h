#ifndef OPERATORS_H
#define OPERATORS_H

#include <string>
#include <map>
#include <algorithm>
#include <utility>
#include <exception>

namespace avm {
enum st_binary_operator {
  bin_opr_invalid = -1,
  opr_power,
  opr_multiply,
  opr_divide,
  opr_floor_divide,
  opr_modulus,
  opr_add,
  opr_subtract,

  opr_bitxor,
  opr_bitand,
  opr_bitor,
  opr_bitand_assign,
  opr_bitxor_assign,
  opr_bitor_assign,
  opr_bitshift_left,
  opr_bitshift_right,

  opr_logand,
  opr_logor,

  opr_equals,
  opr_nequal,
  opr_less,
  opr_greater,
  opr_less_eql,
  opr_greater_eql,

  opr_assignment,
  opr_add_assign,
  opr_subtract_assign,
  opr_multiply_assign,
  opr_divide_assign,
  opr_modulus_assign
};

enum st_unary_operator {
  un_opr_invalid = -1,
  opr_lognot,
  opr_negative,
  opr_positive,
  opr_bitcompl,
  opr_increment,
  opr_decrement,
};

static std::map<std::string, st_binary_operator> binary_operators = {
  { "**", opr_power },
  { "*", opr_multiply },
  { "/", opr_floor_divide },
  { "\\", opr_divide },
  { "%", opr_modulus },
  { "+", opr_add },
  { "-", opr_subtract },

  { "^", opr_bitxor },
  { "&", opr_bitand },
  { "|", opr_bitor },
  { "<<", opr_bitshift_left },
  { ">>", opr_bitshift_right },

  { "&&", opr_logand },
  { "||", opr_logor },
  { "==", opr_equals },
  { "!=", opr_nequal },

  { "<", opr_less },
  { ">", opr_greater },
  { "<=", opr_less_eql },
  { ">=", opr_greater_eql },

  { "=", opr_assignment },
  { "+=", opr_add_assign },
  { "-=", opr_subtract_assign },
  { "*=", opr_multiply_assign },
  { "/=", opr_divide_assign },
  { "%=", opr_modulus_assign },
  { "&=", opr_bitand_assign },
  { "^=", opr_bitxor_assign },
  { "|=", opr_bitor_assign }
};

static std::map<std::string, st_unary_operator> unary_operators = {
  { "!", opr_lognot },
  { "+", opr_positive },
  { "-", opr_negative },
  { "~", opr_bitcompl },
  { "++", opr_increment },
  { "--", opr_decrement },
};

static std::map<st_binary_operator, int> precedence_map = {
  { opr_power, 13 },
  { opr_multiply, 12 },{ opr_divide, 12 },{ opr_floor_divide, 12 },{ opr_modulus, 12 },
  { opr_add, 11 },{ opr_subtract, 11 },
  { opr_bitshift_left, 10 },{ opr_bitshift_right, 10 },
  { opr_greater, 9 },{ opr_greater_eql, 9 },{ opr_less, 9 },{ opr_less_eql, 9 },
  { opr_equals, 8 },{ opr_nequal, 8 },
  { opr_bitand, 7 },
  { opr_bitxor, 6 },
  { opr_bitor, 5 },
  { opr_logand, 4 },
  { opr_logor, 3 },
  { opr_assignment, 2 },
  { opr_add_assign, 2 },{ opr_subtract_assign, 2 },
  { opr_multiply_assign, 2 },{ opr_divide_assign, 2 },{ opr_modulus_assign, 2 },
  { opr_bitand_assign, 2 },{ opr_bitxor_assign, 2 },{ opr_bitor_assign, 2 }
};

inline const std::string &bin_opr_tostr(st_binary_operator op) {
  auto it = find_if(binary_operators.begin(), binary_operators.end(),
    [&op](const std::pair<std::string, st_binary_operator> &item) {
    return item.second == op;
  });

  if (it == binary_operators.end()) {
    throw std::out_of_range("Binary operator not found");
  }
  return it->first;
}

inline const std::string &un_opr_tostr(st_unary_operator op) {
  auto it = find_if(unary_operators.begin(), unary_operators.end(),
    [&op](const std::pair<std::string, st_unary_operator> &item) {
    return item.second == op;
  });

  if (it == unary_operators.end()) {
    throw std::out_of_range("Unary operator not found");
  }
  return it->first;
}

inline const st_binary_operator bin_opr_fromstr(const std::string &str) {
  auto it = binary_operators.find(str);
  if (it == binary_operators.end()) {
    return st_binary_operator::bin_opr_invalid;
  } else {
    return it->second;
  }
}

inline const st_unary_operator un_opr_fromstr(const std::string &str) {
  auto it = unary_operators.find(str);
  if (it == unary_operators.end()) {
    return st_unary_operator::un_opr_invalid;
  } else {
    return it->second;
  }
}

inline bool is_operator(const std::string &str) {
  return un_opr_fromstr(str) != st_unary_operator::un_opr_invalid ||
    bin_opr_fromstr(str) != st_binary_operator::bin_opr_invalid;
}

inline int opr_precedence(st_binary_operator op) {
  return precedence_map.at(op);
}
} // namespace avm

#endif