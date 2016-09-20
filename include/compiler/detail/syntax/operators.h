#ifndef OPERATORS_H
#define OPERATORS_H

#include <string>
#include <map>
#include <algorithm>
#include <utility>
#include <exception>

namespace avm {
enum BinaryOp {
    BinOp_invalid = -1,
    BinOp_power,
    BinOp_multiply,
    BinOp_divide,
    BinOp_floor_divide,
    BinOp_modulus,
    BinOp_add,
    BinOp_subtract,

    BinOp_bitxor,
    BinOp_bitand,
    BinOp_bitor,
    BinOp_bitand_assign,
    BinOp_bitxor_assign,
    BinOp_bitor_assign,
    BinOp_bitshift_left,
    BinOp_bitshift_right,

    BinOp_logand,
    BinOp_logor,

    BinOp_equals,
    BinOp_not_equal,
    BinOp_less,
    BinOp_greater,
    BinOp_less_eql,
    BinOp_greater_eql,

    BinOp_assign,
    BinOp_add_assign,
    BinOp_subtract_assign,
    BinOp_multiply_assign,
    BinOp_divide_assign,
    BinOp_modulus_assign
};

enum UnaryOp {
    UnOp_invalid = -1,
    UnOp_lognot,
    UnOp_negative,
    UnOp_positive,
    UnOp_bitcompl,
    UnOp_increment,
    UnOp_decrement,
};

static std::map<std::string, BinaryOp> binary_operators = {
  { "**", BinOp_power },
  { "*", BinOp_multiply },
  { "/", BinOp_floor_divide },
  { "\\", BinOp_divide },
  { "%", BinOp_modulus },
  { "+", BinOp_add },
  { "-", BinOp_subtract },

  { "^", BinOp_bitxor },
  { "&", BinOp_bitand },
  { "|", BinOp_bitor },
  { "<<", BinOp_bitshift_left },
  { ">>", BinOp_bitshift_right },

  { "&&", BinOp_logand },
  { "||", BinOp_logor },
  { "==", BinOp_equals },
  { "!=", BinOp_not_equal },

  { "<", BinOp_less },
  { ">", BinOp_greater },
  { "<=", BinOp_less_eql },
  { ">=", BinOp_greater_eql },

  { "=", BinOp_assign },
  { "+=", BinOp_add_assign },
  { "-=", BinOp_subtract_assign },
  { "*=", BinOp_multiply_assign },
  { "/=", BinOp_divide_assign },
  { "%=", BinOp_modulus_assign },
  { "&=", BinOp_bitand_assign },
  { "^=", BinOp_bitxor_assign },
  { "|=", BinOp_bitor_assign }
};

static std::map<std::string, UnaryOp> unary_operators = {
  { "!", UnOp_lognot },
  { "+", UnOp_positive },
  { "-", UnOp_negative },
  { "~", UnOp_bitcompl },
  { "++", UnOp_increment },
  { "--", UnOp_decrement },
};

static std::map<BinaryOp, int> precedence_map = {
  { BinOp_power, 13 },
  { BinOp_multiply, 12 },{ BinOp_divide, 12 },{ BinOp_floor_divide, 12 },{ BinOp_modulus, 12 },
  { BinOp_add, 11 },{ BinOp_subtract, 11 },
  { BinOp_bitshift_left, 10 },{ BinOp_bitshift_right, 10 },
  { BinOp_greater, 9 },{ BinOp_greater_eql, 9 },{ BinOp_less, 9 },{ BinOp_less_eql, 9 },
  { BinOp_equals, 8 },{ BinOp_not_equal, 8 },
  { BinOp_bitand, 7 },
  { BinOp_bitxor, 6 },
  { BinOp_bitor, 5 },
  { BinOp_logand, 4 },
  { BinOp_logor, 3 },
  { BinOp_assign, 2 },
  { BinOp_add_assign, 2 },{ BinOp_subtract_assign, 2 },
  { BinOp_multiply_assign, 2 },{ BinOp_divide_assign, 2 },{ BinOp_modulus_assign, 2 },
  { BinOp_bitand_assign, 2 },{ BinOp_bitxor_assign, 2 },{ BinOp_bitor_assign, 2 }
};

static const std::string &BinaryOp_ToString(BinaryOp op)
{
    auto it = find_if(binary_operators.begin(), binary_operators.end(),
        [&op](const std::pair<std::string, BinaryOp> &item)
    {
        return item.second == op;
    });

    if (it == binary_operators.end()) {
        throw std::out_of_range("Binary operator not found");
    }
    return it->first;
}

static const std::string &UnaryOp_ToString(UnaryOp op)
{
    auto it = find_if(unary_operators.begin(), unary_operators.end(),
        [&op](const std::pair<std::string, UnaryOp> &item)
    {
        return item.second == op;
    });

    if (it == unary_operators.end()) {
        throw std::out_of_range("Unary operator not found");
    }
    return it->first;
}

static const BinaryOp BinaryOp_FromString(const std::string &str)
{
    auto it = binary_operators.find(str);
    if (it == binary_operators.end()) {
        return BinaryOp::BinOp_invalid;
    } else {
        return it->second;
    }
}

static const UnaryOp UnaryOp_FromString(const std::string &str)
{
    auto it = unary_operators.find(str);
    if (it == unary_operators.end()) {
        return UnaryOp::UnOp_invalid;
    } else {
        return it->second;
    }
}

static bool IsOperator(const std::string &str)
{
    return UnaryOp_FromString(str) != UnaryOp::UnOp_invalid ||
        BinaryOp_FromString(str) != BinaryOp::BinOp_invalid;
}

static int BinaryOp_Precedence(BinaryOp op)
{
    return precedence_map.at(op);
}
} // namespace avm

#endif