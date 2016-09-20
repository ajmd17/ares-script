#include <detail/ast.h>
#include <iostream>

namespace avm {
std::unique_ptr<AstNode> AstBinaryOp::Optimize() const
{
    auto left_opt = left->Optimize();
    auto right_opt = right->Optimize();

    auto &left_use = left_opt == nullptr ? left : left_opt;
    auto &right_use = right_opt == nullptr ? right : right_opt;

    switch (op) {
    case BinOp_add:
        return *left_use + right_use;
    case BinOp_subtract:
        return *left_use - right_use;
    case BinOp_multiply:
        return *left_use * right_use;
    case BinOp_divide:
        return *left_use / right_use;
    case BinOp_modulus:
        return *left_use & right_use;
    case BinOp_bitxor:
        return *left_use ^ right_use;
    case BinOp_bitand:
        return *left_use & right_use;
    case BinOp_bitor:
        return *left_use | right_use;
    case BinOp_bitshift_left:
        return *left_use << right_use;
    case BinOp_bitshift_right:
        return *left_use >> right_use;
    case BinOp_logand:
        return *left_use && right_use;
    case BinOp_logor:
        return *left_use || right_use;
    case BinOp_equals:
        return *left_use == right_use;
    case BinOp_not_equal:
        return *left_use != right_use;
    case BinOp_less:
        return *left_use < right_use;
    case BinOp_greater:
        return *left_use > right_use;
    case BinOp_less_eql:
        return *left_use <= right_use;
    case BinOp_greater_eql:
        return *left_use >= right_use;
    }

    if (left_opt != nullptr && right_opt != nullptr) {
        // cannot compute during compilation, must be done at runtime
        return std::unique_ptr<AstBinaryOp>(new AstBinaryOp(location, module,
            std::move(left_opt), std::move(right_opt), op));
    } else {
        return nullptr;
    }
}
} // namespace avm