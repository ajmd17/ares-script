#include <detail/ast.h>

namespace avm {
// returns nullptr if failure
std::unique_ptr<AstNode> AstNode::Optimize() const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator+(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator-(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator*(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator/(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator%(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator^(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator&(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator|(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator<<(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator>>(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator&&(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator||(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator==(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator!=(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator<(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator>(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator<=(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator>=(const std::unique_ptr<AstNode> &other) const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator!() const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator~() const
{
    return nullptr;
}

std::unique_ptr<AstNode> AstNode::operator-() const
{
    return nullptr;
}
}