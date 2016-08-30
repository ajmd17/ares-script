#include <detail/ast.h>

namespace avm {
std::unique_ptr<AstNode> AstExpression::Optimize() const {
  auto child_opt = child->Optimize();
  if (child_opt == nullptr) {
    return nullptr;
  } else {
    return std::unique_ptr<AstExpression>(new AstExpression(location, module,
      std::move(child_opt), should_clear_stack));
  }
}
} // namespace avm