#include <detail/ast.h>

namespace avm {
std::unique_ptr<AstNode> AstFloat::operator+(const std::unique_ptr<AstNode> &rhs) const
{
    switch (rhs->type) {
    case ast_type_integer:
        // promote int to float
        return std::unique_ptr<AstFloat>(new AstFloat(location, module,
            value + static_cast<const AstInteger*>(rhs.get())->value));
    case ast_type_float:
        return std::unique_ptr<AstFloat>(new AstFloat(location, module,
            value + static_cast<const AstFloat*>(rhs.get())->value));
    }
    return nullptr;
}

std::unique_ptr<AstNode> AstFloat::operator-(const std::unique_ptr<AstNode> &rhs) const
{
    switch (rhs->type) {
    case ast_type_integer:
        // promote int to float
        return std::unique_ptr<AstFloat>(new AstFloat(location, module,
            value - static_cast<const AstInteger*>(rhs.get())->value));
    case ast_type_float:
        return std::unique_ptr<AstFloat>(new AstFloat(location, module,
            value - static_cast<const AstFloat*>(rhs.get())->value));
    }
    return nullptr;
}

std::unique_ptr<AstNode> AstFloat::operator*(const std::unique_ptr<AstNode> &rhs) const
{
    switch (rhs->type) {
    case ast_type_integer:
        // promote int to float
        return std::unique_ptr<AstFloat>(new AstFloat(location, module,
            value * static_cast<const AstInteger*>(rhs.get())->value));
    case ast_type_float:
        return std::unique_ptr<AstFloat>(new AstFloat(location, module,
            value * static_cast<const AstFloat*>(rhs.get())->value));
    }
    return nullptr;
}

std::unique_ptr<AstNode> AstFloat::operator/(const std::unique_ptr<AstNode> &rhs) const
{
    switch (rhs->type) {
    case ast_type_integer:
        // promote int to float
        return std::unique_ptr<AstFloat>(new AstFloat(location, module,
            value / static_cast<const AstInteger*>(rhs.get())->value));
    case ast_type_float:
        return std::unique_ptr<AstFloat>(new AstFloat(location, module,
            value / static_cast<const AstFloat*>(rhs.get())->value));
    }
    return nullptr;
}

std::unique_ptr<AstNode> AstFloat::operator==(const std::unique_ptr<AstNode> &rhs) const
{
    switch (rhs->type) {
    case ast_type_integer:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value == static_cast<const AstInteger*>(rhs.get())->value));
    case ast_type_float:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value == static_cast<const AstFloat*>(rhs.get())->value));
    }
    return nullptr;
}

std::unique_ptr<AstNode> AstFloat::operator!=(const std::unique_ptr<AstNode> &rhs) const
{
    switch (rhs->type) {
    case ast_type_integer:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value != static_cast<const AstInteger*>(rhs.get())->value));
    case ast_type_float:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value != static_cast<const AstFloat*>(rhs.get())->value));
    }
    return nullptr;
}

std::unique_ptr<AstNode> AstFloat::operator<(const std::unique_ptr<AstNode> &rhs) const
{
    switch (rhs->type) {
    case ast_type_integer:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value < static_cast<const AstInteger*>(rhs.get())->value));
    case ast_type_float:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value < static_cast<const AstFloat*>(rhs.get())->value));
    }
    return nullptr;
}

std::unique_ptr<AstNode> AstFloat::operator>(const std::unique_ptr<AstNode> &rhs) const
{
    switch (rhs->type) {
    case ast_type_integer:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value > static_cast<const AstInteger*>(rhs.get())->value));
    case ast_type_float:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value > static_cast<const AstFloat*>(rhs.get())->value));
    }
    return nullptr;
}

std::unique_ptr<AstNode> AstFloat::operator<=(const std::unique_ptr<AstNode> &rhs) const
{
    switch (rhs->type) {
    case ast_type_integer:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value <= static_cast<const AstInteger*>(rhs.get())->value));
    case ast_type_float:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value <= static_cast<const AstFloat*>(rhs.get())->value));
    }
    return nullptr;
}

std::unique_ptr<AstNode> AstFloat::operator>=(const std::unique_ptr<AstNode> &rhs) const
{
    switch (rhs->type) {
    case ast_type_integer:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value >= static_cast<const AstInteger*>(rhs.get())->value));
    case ast_type_float:
        // demote to integer
        return std::unique_ptr<AstInteger>(new AstInteger(location, module,
            value >= static_cast<const AstFloat*>(rhs.get())->value));
    }
    return nullptr;
}

std::unique_ptr<AstNode> AstFloat::operator!() const
{
    // demote to integer
    return std::unique_ptr<AstInteger>(new AstInteger(location, module, !value));
}

std::unique_ptr<AstNode> AstFloat::operator-() const
{
    return std::unique_ptr<AstFloat>(new AstFloat(location, module, -value));
}
} // namespace avm