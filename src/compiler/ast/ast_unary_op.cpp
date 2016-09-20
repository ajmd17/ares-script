#include <detail/ast.h>

namespace avm {
std::unique_ptr<AstNode> AstUnaryOp::Optimize() const
{
    auto child_opt = child->Optimize();
    auto &child_use = child_opt == nullptr ? child : child_opt;

    switch (op) {
    case UnOp_lognot:
        return !(*child_use);
    case UnOp_bitcompl:
        return ~(*child_use);
    case UnOp_negative:
        return -(*child_use);
    }

    if (child_opt != nullptr) {
        // cannot compute during compilation, must be done at runtime
        return std::unique_ptr<AstUnaryOp>(new AstUnaryOp(location, module,
            std::move(child_opt), op));
    } else {
        return nullptr;
    }
}
} // namespace avm