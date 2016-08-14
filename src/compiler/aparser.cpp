#include <aparser.h>
#include <config.h>

#include <iostream>
#include <cctype>

namespace avm {
Parser::Parser(std::vector<Token> tokens, LexerState lexer_state) {
  state.tokens = tokens;
  state.errors = lexer_state.errors;
  filepath = lexer_state.location.file;
}

std::unique_ptr<AstModule> Parser::parse() {
  std::unique_ptr<AstModule> unit = nullptr;
  std::string module_name = "__anon";

  if (!MatchRead(tk_keyword, keyword_tostr(kw_module))) {
    ErrorMsg(msg_expected_module, Location());
  } else {
    Token *ident = ExpectRead(tk_identifier);
    if (ident != nullptr) {
      module_name = ident->value;

      if (std::islower(module_name[0])) {
        InfoMsg(msg_module_name_begins_lowercase, ident->location, module_name);
      }

      unit = std::unique_ptr<AstModule>(new AstModule(Location(), module_name));
      main_module = unit.get();
      while (state.position < state.tokens.size()) {
        unit->AddChild(ParseStatement());
      }

      return std::move(unit);
    }
  }

  return nullptr;
}

SourceLocation Parser::Location() {
  if (!state.tokens.empty()) {
    Token *token = Peek();
    return (token ? token->location : Peek(-1)->location);
  } else {
    return SourceLocation({ 0, 0, filepath });
  }
}

Token *Parser::Peek(int n) {
  if (state.position + n < state.tokens.size()) {
    return &state.tokens[state.position + n];
  }

  return nullptr;
}

Token *Parser::Read() {
  if (state.position >= state.tokens.size()) {
    return nullptr;
  } else {
    return &state.tokens[state.position++];
  }
}

bool Parser::Match(TokenType type, int n) {
  return (Peek(n) && Peek(n)->type == type);
}

bool Parser::Match(TokenType type, const std::string &str) {
  return (Match(type) && Peek()->value == str);
}

bool Parser::MatchRead(TokenType type) {
  if (Match(type)) {
    Read();
    return true;
  }
  return false;
}

bool Parser::MatchRead(TokenType type, Token *&token) {
  if (Match(type)) {
    token = Read();
    return true;
  }
  return false;
}

bool Parser::MatchRead(TokenType type, const std::string &str) {
  if (Match(type, str)) {
    Read();
    return true;
  }
  return false;
}

Token *Parser::ExpectRead(TokenType type) {
  Token *tok = nullptr;

  if (!MatchRead(type, tok)) {
    Token *bad = Read();
    if (bad) {
      switch (type) {
      case tk_identifier:
        ErrorMsg(msg_expected_identifier, bad->location);
        break;
        // FALLTHROUGH
      case tk_open_parenthesis:
      case tk_close_parenthesis:
      case tk_open_brace:
      case tk_close_brace:
      case tk_open_bracket:
      case tk_close_bracket:
      case tk_semicolon:
      case tk_colon:
      case tk_comma:
        ErrorMsg(msg_expected_token, bad->location, Token::TokenTypeToString(type));
        break;
      default:
        ErrorMsg(msg_unexpected_token, bad->location, bad->value);
        break;
      }
    } else {
      ErrorMsg(msg_unexpected_eof, Location());
    }
  }
  return tok;
}

Token *Parser::ExpectRead(TokenType type, const std::string &str) {
  Token *tok = Peek();

  if (!MatchRead(type, str)) {
    Token *bad = Read();
    if (bad) {
      ErrorMsg(msg_expected_token, bad->location, str);
    } else {
      ErrorMsg(msg_unexpected_eof, Location());
    }
  }

  return tok;
}

int Parser::OpPrecedence() {
  auto *current = Peek();
  if (!(current && current->type == tk_operator)) {
    return -1;
  }

  st_binary_operator op = bin_opr_fromstr(current->value);
  if (op == st_binary_operator::bin_opr_invalid) {
    ErrorMsg(msg_illegal_operator, current->location, current->value);
  }
  return opr_precedence(op);
}

/** An import statement allows the inclusion of external code.
    It will attempt to find the file that is being referenced,
    and through an error if it cannot be found. Files are only
    included once for the active build.

    Import statements may be grouped together within a set of
    curly braces, for efficiency.

    Example:
    1.  import {
          'some_library',
          'another_library', // <-- final comma optional
        }

    2.  import 'some_library';
        import 'another_library';

*/
std::unique_ptr<AstNode> Parser::ParseImports() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_import));

  // imports can be declared within a set of braces, or on a single line
  if (!MatchRead(tk_open_brace)) {
    return ParseImport();
  }

  std::vector<std::unique_ptr<AstNode>> imports;
  while (Peek()) {
    if (MatchRead(tk_close_brace)) {
      break;
    }
    imports.push_back(ParseImport());
    if (MatchRead(tk_close_brace) || !ExpectRead(tk_comma)) {
      break;
    }
  }

  return std::move(std::unique_ptr<AstImports>(new AstImports(tok->location, main_module, std::move(imports))));
}

std::unique_ptr<AstNode> Parser::ParseImport() {
  const std::string local_path =
    filepath.substr(0, filepath.find_last_of("/\\")) + "/";

  bool is_module_import = true;
  std::string value;

  Token *tok = nullptr;
  if (MatchRead(tk_string, tok)) {
    is_module_import = false;
    value = tok->value;
  } else if (MatchRead(tk_identifier, tok)) {
    is_module_import = true;
    value = tok->value;
  } else {
    ErrorMsg(msg_unexpected_token, Location(), Read()->value);
    return nullptr;
  }

  if ((local_path + value) == filepath) {
    ErrorMsg(ErrorType::msg_import_current_file, tok->location);
    return nullptr;
  }

  return std::move(std::unique_ptr<AstImport>(new AstImport(tok->location, main_module, value,
    local_path, is_module_import)));
}

std::unique_ptr<AstNode> Parser::ParseStatement() {
  if (MatchRead(tk_semicolon)) {
    return std::move(std::unique_ptr<AstStatement>(new AstStatement(Location(), main_module)));
  }

  std::unique_ptr<AstNode> node = nullptr;
  if (Match(tk_keyword)) {
    auto *before = Peek(-1);
    if (!before ||
      !(before->type == tk_semicolon ||
        before->type == tk_open_brace ||
        before->type == tk_close_brace ||
        before->type == tk_colon ||
        before->type == tk_period ||
        before->type == tk_right_arrow ||
        before->type == tk_left_arrow)) {
      WarningMsg(msg_expected_semicolon, before->location);
    }

    const std::string &val = Peek()->value;

    if (val == keyword_tostr(kw_var)) {
      node = ParseVariableDeclaration();
    } else if (val == keyword_tostr(kw_alias)) {
      node = ParseAlias();
    } else if (val == keyword_tostr(kw_using)) {
      node = ParseUsing();
    } else if (val == keyword_tostr(kw_attribute)) {
      node = ParseAttribute();
    } else if (val == keyword_tostr(kw_class)) {
      node = ParseClass();
    } else if (val == keyword_tostr(kw_enum)) {
      node = ParseEnum();
    } else if (val == keyword_tostr(kw_import)) {
      node = ParseImports();
    } else if (val == keyword_tostr(kw_func)) {
      node = ParseFunctionDefinition();
    } else if (val == keyword_tostr(kw_if)) {
      node = ParseIfStmt();
    } else if (val == keyword_tostr(kw_return)) {
      node = ParseReturnStmt();
    } else if (val == keyword_tostr(kw_for)) {
      node = ParseForLoop();
    } else if (val == keyword_tostr(kw_while)) {
      node = ParseWhileLoop();
    } else if (val == keyword_tostr(kw_print)) {
      node = ParsePrintStmt();
    } else if (val == keyword_tostr(kw_else)) {
      ErrorMsg(msg_else_outside_if, Location());
      Read();
    } else if (val == keyword_tostr(kw_try)) {
      node = ParseTryCatch();
    } else {
      // keyword not handled
      ErrorMsg(msg_internal_error, Location());
    }
    /*else if (val == getKeyword(KW_FOREACH))
    return parseForEach();
    else if (val == getKeyword(KW_BREAK))
    return parseBreak();
    else if (val == getKeyword(KW_CONTINUE))
    return parseContinue();
    else if (val == getKeyword(KW_DO))
    return parseDoWhile();
    else if (val == getKeyword(KW_IMPORT))
    return parseImport();
    else if (val == getKeyword(KW_THROW))
    return parseThrow();
    else if (val == getKeyword(KW_TRY))
    return parseTry();
    else if (val == getKeyword(KW_SUPER))
    return parseSuper();*/
  } else if (Match(tk_open_brace)) {
    node = ParseCodeBlock();
  } else {
    // finally try parsing as an expression
    node = ParseExpression(true);
    if (!node) {
      ErrorMsg(msg_illegal_expression, Location());
    }
  }

  MatchRead(tk_semicolon);

  return std::move(node);
}

/** Variable declarations work like so:
      var x;
      var y;

    These variables will be set to null by default.
    Optionally, you may specify an assignment for the variable.

    Example:
      var x = 3;
      -- or --
      var x: 3;
*/
std::unique_ptr<AstNode> Parser::ParseVariableDeclaration() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_var));

  if ((!Peek()) || (Peek()->type != tk_identifier)) {
    ErrorMsg(msg_expected_identifier, Location());
  } else {
    Token *ident_token = ExpectRead(tk_identifier);
    std::string identifier = ident_token->value;

    if (std::isupper(identifier[0])) {
      InfoMsg(msg_variable_name_begins_uppercase, ident_token->location, identifier);
    }

    variable_names.push(identifier);

    std::unique_ptr<AstNode> assignment = nullptr;
    if (MatchRead(tk_operator, bin_opr_tostr(opr_assignment)) || MatchRead(tk_colon)) {
      assignment = ParseExpression();
    } else {
      // load null value in place of an expression
      assignment = std::unique_ptr<AstNull>(new AstNull(Location(), main_module));
    }

    variable_names.pop();

    return std::move(std::unique_ptr<AstVariableDeclaration>(new AstVariableDeclaration(tok->location, main_module,
      identifier, std::move(assignment), /*not const*/ false)));
  }

  return nullptr;
}

std::unique_ptr<AstNode> Parser::ParseAlias() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_alias));

  if ((!Peek()) || (Peek()->type != tk_identifier)) {
    ErrorMsg(msg_expected_identifier, Location());
  } else {
    std::string identifier = Read()->value;
    if (MatchRead(tk_operator, bin_opr_tostr(opr_assignment)) || MatchRead(tk_colon)) {
      std::unique_ptr<AstNode> alias_to = nullptr;
      if (Match(TokenType::tk_identifier) && (alias_to = ParseIdentifier())) {
        return std::move(std::unique_ptr<AstAlias>(new AstAlias(tok->location, main_module, identifier, std::move(alias_to))));
      } else {
        ErrorMsg(ErrorType::msg_alias_must_be_identifier, tok->location, identifier);
      }
    } else {
      ErrorMsg(ErrorType::msg_alias_missing_assignment, tok->location, identifier);
    }
  }
  return nullptr;
}

/** "using" is just sugar for an alias declaration. "using module" generates an alias
    for each identifier in the chosen module.

    Example:
      using FileIO.open;
      open(...);
*/
std::unique_ptr<AstNode> Parser::ParseUsing() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_using));

  if (MatchRead(TokenType::tk_keyword, keyword_tostr(kw_module))) {
    std::string identifier; // get the identifier after "using".
    for (int i = 0; Peek(i) && Peek(i)->type == TokenType::tk_identifier;) {
      identifier = Read()->value; ++i;
      if (Peek(i) && MatchRead(TokenType::tk_period)) {
        ++i;
      }
    }
    return std::move(std::unique_ptr<AstUseModule>(new AstUseModule(tok->location, main_module, identifier)));
  } else if (Match(TokenType::tk_identifier)) {
    std::string identifier; // get the identifier after "using".
    for (int i = 0; Peek(i) && Peek(i)->type == TokenType::tk_identifier;) {
      identifier = Peek(i++)->value;
      if (Peek(i) && Peek(i)->type == TokenType::tk_period) {
        ++i;
      }
    }
    return std::move(std::unique_ptr<AstAlias>(new AstAlias(tok->location, main_module, identifier, std::move(ParseIdentifier()))));
  } else {
    ErrorMsg(msg_expected_identifier, Location());
  }
  return nullptr;
}

std::unique_ptr<AstNode> Parser::ParseAttribute() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_attribute));
  std::vector<std::string> attributes;

  ExpectRead(tk_open_brace);
  while (Match(tk_string)) {
    attributes.push_back(Read()->value);
    if (!MatchRead(tk_comma)) {
      break;
    }
  }
  ExpectRead(tk_close_brace);

  auto stmt = ParseStatement();
  stmt->attributes = attributes;
  return std::move(stmt);
}

std::unique_ptr<AstNode> Parser::ParseBinaryOp(int expr_prec, std::unique_ptr<AstNode> left) {
  while (true) {
    int tok_prec = OpPrecedence();
    if (tok_prec < expr_prec)
      return left;

    Token *tok = ExpectRead(tk_operator);
    st_binary_operator op = bin_opr_fromstr(tok->value);
    if (op == st_binary_operator::bin_opr_invalid) {
      ErrorMsg(msg_illegal_operator, tok->location, tok->value);
    }

    auto right = ParseTerm();
    if (!right) {
      return nullptr;
    }

    int next_prec = OpPrecedence();
    if (tok_prec < next_prec) {
      right = ParseBinaryOp(tok_prec + 1, move(right));
      if (!right)
        return nullptr;
    }

    left = std::unique_ptr<AstBinaryOp>(new AstBinaryOp(tok->location, main_module,
      std::move(left), std::move(right), op));
  }

  return nullptr;
}

std::unique_ptr<AstNode> Parser::ParseUnaryOp() {
  Token *tok = ExpectRead(tk_operator);
  st_unary_operator op = un_opr_fromstr(tok->value);
  if (op == st_unary_operator::un_opr_invalid) {
    ErrorMsg(msg_illegal_operator, tok->location, tok->value);
  }

  auto value = ParseTerm();
  if (!value) {
    return nullptr;
  }

  return std::move(std::unique_ptr<AstUnaryOp>(new AstUnaryOp(tok->location, main_module,
    std::move(value), op)));
}

std::unique_ptr<AstNode> Parser::ParseClass() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_class));
  Token *ident = ExpectRead(tk_identifier);
  std::vector<std::unique_ptr<AstNode>> members;

  ExpectRead(tk_open_brace);
  while (!MatchRead(tk_close_brace)) {
    members.push_back(std::move(ParseStatement()));
  }

  return std::move(std::unique_ptr<AstClass>(new AstClass(tok->location, main_module,
    ident->value, std::move(members))));
}

std::unique_ptr<AstNode> Parser::ParseEnum() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_enum));
  Token *ident = ExpectRead(tk_identifier);
  std::vector<std::pair<std::string, std::unique_ptr<AstInteger>>> members;

  AVMInteger_t start_value = 0;

  ExpectRead(tk_open_brace);
  while (Peek()) {
    if (MatchRead(tk_close_brace)) {
      break;
    }
    
    Token *key = ExpectRead(tk_identifier);
    if (MatchRead(tk_operator, bin_opr_tostr(opr_assignment)) || MatchRead(tk_colon)) {
      Token *value = ExpectRead(tk_integer);
      start_value = atoi(value->value.c_str());
    }
    members.push_back({key->value, std::unique_ptr<AstInteger>(new AstInteger(key->location, main_module, start_value))});
    ++start_value;

    if (MatchRead(tk_close_brace) || !ExpectRead(tk_comma)) {
      break;
    }
  }

  return std::move(std::unique_ptr<AstEnum>(new AstEnum(tok->location, 
    main_module, ident->value, std::move(members))));
}

std::unique_ptr<AstNode> Parser::ParseParenthesis() {
  Read();
  auto expr = ParseExpression();
  if (!expr) {
    return nullptr;
  }
  ExpectRead(tk_close_parenthesis);
  return expr;
}

std::unique_ptr<AstNode> Parser::ParseIntegerLiteral() {
  Token *tok = ExpectRead(tk_integer);
  AVMInteger_t value = (AVMInteger_t)atoll(tok->value.c_str());

  return std::move(std::unique_ptr<AstInteger>(new AstInteger(tok->location, main_module, value)));
}

std::unique_ptr<AstNode> Parser::ParseFloatLiteral() {
  Token *tok = ExpectRead(tk_float);
  AVMFloat_t value = (AVMFloat_t)atof(tok->value.c_str());

  return std::move(std::unique_ptr<AstFloat>(new AstFloat(tok->location, main_module, value)));
}

/** An identifier may be a variable name, a function call, or
    any alphanumeric name.

    Member access is also detected while parsing the identifier,
    so 'myclass.name' would also be parsed here.

    Example:
        some_identifier = 3;
        some_function();
*/
std::unique_ptr<AstNode> Parser::ParseIdentifier() {
  // check for semicolon
  auto *before = Peek(-1);
  if (before &&
    !(before->type == tk_semicolon ||
      before->type == tk_operator ||
      before->type == tk_keyword ||
      before->type == tk_open_brace ||
      before->type == tk_close_brace ||
      before->type == tk_open_bracket ||
      before->type == tk_open_parenthesis ||
      before->type == tk_period ||
      before->type == tk_colon ||
      before->type == tk_comma ||
      before->type == tk_right_arrow)) {
    WarningMsg(msg_expected_semicolon, before->location);
  }

  std::unique_ptr<AstNode> result = nullptr;
  Token *tok = ExpectRead(tk_identifier);
  std::string identifier = tok->value;
  if (MatchRead(tk_open_parenthesis)) {
    // function call
    std::vector<std::unique_ptr<AstNode>> arguments;

    if (Peek()->type != tk_close_parenthesis) {
      while (true) {
        auto arg = ParseExpression();
        if (!arg) {
          return nullptr;
        }
        arguments.push_back(std::move(arg));

        if (Peek()->type == tk_close_parenthesis) {
          break;
        } else if (Peek()->type != tk_comma) {
          ErrorMsg(msg_unexpected_token, Location(), Peek()->value);
          return nullptr;
        }

        Read(); // read comma
      }
    }

    Read(); // read clone parenthesis

    result = std::unique_ptr<AstFunctionCall>(new AstFunctionCall(tok->location, main_module,
      identifier, std::move(arguments)));
  } else {
    result = std::unique_ptr<AstVariable>(new AstVariable(tok->location, main_module, identifier));
  }

  // check for array indexing
  if (MatchRead(tk_open_bracket)) {
    /// \todo implementation
    auto idx = std::move(ParseExpression());
    ExpectRead(tk_close_bracket);
    result = std::unique_ptr<AstArrayAccess>(new AstArrayAccess(tok->location, main_module,
      std::move(result), std::move(idx)));
  }

  // check for member access (something.whatever) after
  if (MatchRead(tk_period)) {
    std::unique_ptr<AstNode> next = nullptr;

    if (Match(tk_identifier)) {
      next = std::move(ParseIdentifier());
    } else {
      ErrorMsg(msg_expected_identifier, Location());
      Read();
    }

    result = std::unique_ptr<AstMemberAccess>(new AstMemberAccess(tok->location, main_module,
      identifier, std::move(result), std::move(next)));
  }

  return result;
}

std::unique_ptr<AstNode> Parser::ParseStringLiteral() {
  std::unique_ptr<AstNode> result = nullptr;
  Token *tok = ExpectRead(tk_string);
  std::string value = tok->value;

  return std::unique_ptr<AstString>(new AstString(tok->location, main_module, value));
}

std::unique_ptr<AstNode> Parser::ParseTrue() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_true));

  return std::unique_ptr<AstTrue>(new AstTrue(tok->location, main_module));
}

std::unique_ptr<AstNode> Parser::ParseFalse() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_false));

  return std::unique_ptr<AstFalse>(new AstFalse(tok->location, main_module));
}

std::unique_ptr<AstNode> Parser::ParseNull() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_null));

  return std::unique_ptr<AstNull>(new AstNull(tok->location, main_module));
}

/** The 'self' object refers to the current object, and may be
    used within member functions of a class.

    Example:
        self.x = 4;
*/
std::unique_ptr<AstNode> Parser::ParseSelf() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_self));
  std::string identifier = tok->value;
  std::unique_ptr<AstNode> result = std::unique_ptr<AstSelf>(new AstSelf(tok->location, main_module));

  if (MatchRead(tk_period)) {
    std::unique_ptr<AstNode> next = nullptr;

    std::string left_str = Peek()->value;
    if (Match(tk_identifier)) {
      next = std::move(ParseIdentifier());
    } else {
      ErrorMsg(msg_unexpected_token, Location(), Read()->value);
    }

    result = std::unique_ptr<AstMemberAccess>(new AstMemberAccess(tok->location, main_module, left_str,
      std::move(result), std::move(next)));
  }

  return result;
}

/** The 'new' operator allows the creation of a new class instance

    Example:
        var date = new Date();
*/
std::unique_ptr<AstNode> Parser::ParseNew(const std::string &identifier) {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_new));

  std::unique_ptr<AstNode> cls;
  if (Match(tk_identifier)) {
    cls = std::move(ParseIdentifier());
  } else {
    ErrorMsg(msg_unexpected_token, Location(), Read()->value);
  }

  return std::unique_ptr<AstNew>(new AstNew(tok->location, main_module,
    identifier, std::move(cls)));
}

std::unique_ptr<AstNode> Parser::ParseTerm() {
  std::unique_ptr<AstNode> term = nullptr;

  if (!Peek()) {
    Read();
    return nullptr;
  }

  if (Match(tk_open_parenthesis))
    term = move(ParseParenthesis());
  else if (Match(tk_integer))
    term = move(ParseIntegerLiteral());
  else if (Match(tk_float))
    term = move(ParseFloatLiteral());
  else if (Match(tk_identifier))
    term = move(ParseIdentifier());
  else if (Match(tk_string))
    term = move(ParseStringLiteral());
  else if (Match(tk_keyword, keyword_tostr(kw_true)))
    term = move(ParseTrue());
  else if (Match(tk_keyword, keyword_tostr(kw_false)))
    term = move(ParseFalse());
  else if (Match(tk_keyword, keyword_tostr(kw_null)))
    term = move(ParseNull());
  else if (Match(tk_keyword, keyword_tostr(kw_self)))
    term = move(ParseSelf());
  else if (Match(tk_keyword, keyword_tostr(kw_new)))
    term = move(ParseNew(variable_names.top()));
  else if (Match(tk_keyword, keyword_tostr(kw_func)))
    term = move(ParseFunctionExpression());
  else if (Match(tk_operator))
    term = move(ParseUnaryOp());
  else
    ErrorMsg(msg_unexpected_token, Location(), Read()->value);

  return term;
}

std::unique_ptr<AstNode> Parser::ParseExpression(bool pop_after) {
  auto term = ParseTerm();
  if (!term) {
    return nullptr;
  }

  if (Match(tk_operator)) {
    // parse binary expression
    auto bin_op = ParseBinaryOp(0, std::move(term));
    if (!bin_op) {
      return nullptr;
    }

    term = std::move(bin_op);
  }

  return std::unique_ptr<AstExpression>(new AstExpression(Location(), main_module,
    std::move(term), pop_after));
}

std::unique_ptr<AstNode> Parser::ParseCodeBlock() {
  Token *tok = ExpectRead(tk_open_brace);
  auto block = std::unique_ptr<AstBlock>(new AstBlock(tok->location, main_module));
  while (Peek() && !MatchRead(tk_close_brace)) {
    block->AddChild(ParseStatement());
  }

  return std::move(block);
}

/** Function definitons start with the "func" keyword, with the
    name of the function following after. The parameters of
    the function are enclosed in parenthesis, after the function name.
    Functions taking no parameters do not require parenthesis.

    Overloaded functions are not supported, as functions may be treated
    the same as variables, and even be assigned to variables. This would
    confuse the compiler and the virtual machine as to what variable
    is being referred to.

    The body of a function is enclosed in curly braces, or if it is a single
    line function, it will be anything after a right arrow. If no return
    statement is supplied, one will be automatically added, returning
    null.

    \todo: Make functions return the last statement in the body (à la Rust)

    Example:
        func add_numbers(a, b) {
          return a+b;
        }

        func subtract_numbers(a,b) -> a - b; // \todo implicit return value

        func print_hi
          -> print 'hi\n';

        var c = add_numbers(6,7);
        var d = subtract_numbers(3,4);
        print_hi();
*/
std::unique_ptr<AstNode> Parser::ParseFunctionDefinition() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_func));

  Token *ident_token = ExpectRead(tk_identifier);
  if (!ident_token) {
    return nullptr;
  }

  std::string identifier = ident_token->value;
  if (std::isupper(identifier[0])) {
    InfoMsg(msg_function_name_begins_uppercase, ident_token->location, identifier);
  }

  std::vector<std::string> arguments;
  bool variadic = false;

  if (MatchRead(tk_open_parenthesis)) {
    while (true) {
      Token *tok = nullptr;
      if (MatchRead(tk_identifier, tok)) {
        if (variadic) {
          ErrorMsg(msg_argument_after_varargs, Location());
        }

        arguments.push_back(tok->value);

        if (MatchRead(tk_ellipsis)) {
          variadic = true;
        }

        if (!MatchRead(tk_comma)) {
          break;
        }
      } else {
        break;
      }
    }
    ExpectRead(tk_close_parenthesis);
  }

  std::unique_ptr<AstNode> block = nullptr;
  if (Peek() && Peek()->type == tk_open_brace) {
    block = ParseStatement(); // read the block
  } else {
    ExpectRead(tk_right_arrow);
    // generate a function body from a statement
    auto block_ast = std::unique_ptr<AstBlock>(new AstBlock(Location(), main_module));
    block_ast->children.push_back(ParseStatement());
    block = std::move(block_ast);
  }

  return std::unique_ptr<AstFunctionDefinition>(new AstFunctionDefinition(tok->location, main_module,
    identifier, arguments, std::move(block)));
}

/** A function expression is a function in the form of a variable.
    Typically, global functions are included in the top of a bytecode
    file by the compiler (nested functions are basically function expressions).
    This behavior is to let the user not have to pre-declare functions.
    A function expression has the same scope as a normal variable.

    \todo Let user not have to pre-declare function prototypes

    Example:
      var lambda = func
        -> return 2*2;
      print '2*2 = ', lambda(), '\n';

    \todo Syntax similar to this for efficient lambda declaration: var lambda => return x;
*/
std::unique_ptr<AstNode> Parser::ParseFunctionExpression() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_func));

  std::vector<std::string> arguments;
  if (MatchRead(tk_colon)) {
    while (true) {
      Token *tok = ExpectRead(tk_identifier);
      if (!tok) {
        return nullptr;
      }
      arguments.push_back(tok->value);
      if (Peek()->type != tk_comma) {
        break;
      } else {
        Read(); // read the comma
      }
    }
  }

  std::unique_ptr<AstNode> block = nullptr;
  if (Peek() && Peek()->type == tk_open_brace) {
    block = ParseStatement(); // read the block
  } else {
    ExpectRead(tk_right_arrow);
    // generate a function body from a statement
    auto block_ast = std::unique_ptr<AstBlock>(new AstBlock(Location(), main_module));
    block_ast->children.push_back(ParseStatement());
    block = std::move(block_ast);
  }

  return std::unique_ptr<AstFunctionExpression>(new AstFunctionExpression(tok->location, main_module,
    arguments, std::move(block)));
}

/** If statements do not require parenthesis around the condition.
    The body of the if statement may be within a set of curly braces,
    or may be a single statement after a colon.

    Example:
      if x == 3:
        print 'x is equal to 3';
*/
std::unique_ptr<AstNode> Parser::ParseIfStmt() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_if));
  auto condition = ParseExpression();

  std::unique_ptr<AstNode> if_block = nullptr;
  if (Peek() && Peek()->type == tk_open_brace) {
    if_block = ParseStatement(); // read the block
  } else if (MatchRead(tk_colon)) {
    auto block_ast = std::unique_ptr<AstBlock>(new AstBlock(Location(), main_module));
    auto stmt = ParseStatement();
    if (stmt != nullptr) {
      block_ast->children.push_back(std::move(stmt));
    }
    if_block = std::move(block_ast);
  } else if (MatchRead(tk_semicolon)) {
    // if nothing else, we need a semicolon
    if_block = std::unique_ptr<AstBlock>(new AstBlock(Location(), main_module));
  } else {
    ErrorMsg(msg_unexpected_token, Location(), Read()->value);
  }

  std::unique_ptr<AstNode> else_block = nullptr;
  if (MatchRead(tk_keyword, keyword_tostr(kw_else))) {
    if (Peek() && Peek()->type == tk_open_brace) {
      else_block = ParseStatement(); // read the block
    } else if (MatchRead(tk_colon)) {
      auto block_ast = std::unique_ptr<AstBlock>(new AstBlock(Location(), main_module));
      auto stmt = ParseStatement();
      if (stmt != nullptr) {
        block_ast->children.push_back(std::move(stmt));
      }
      else_block = std::move(block_ast);
    } else if (MatchRead(tk_semicolon)) {
      // if nothing else, we need a semicolon
      else_block = std::unique_ptr<AstBlock>(new AstBlock(Location(), main_module));
    } else {
      ErrorMsg(msg_unexpected_token, Location(), Read()->value);
    }
  }

  return std::unique_ptr<AstIfStmt>(new AstIfStmt(tok->location, main_module,
    std::move(condition), std::move(if_block), std::move(else_block)));
}

/** Print statements can take a variable number of arguments,
    and accept any data type that can be internally converted to a string
    within the virtual machine. They may be surrounded by round brackets to
    appear as a normal function, but that is optional.

    Example:
        print x, y;
        print 4 + x, y, 2, 'hello world';
        print '\n';
        print('hello world\n');

    However, if you intend to put parenthesis before the first argument,
    you must also put them around the entire print statement, like so:
        print((x + 2) * 4);
    Rather than:
        print (x + 2) * 4; // Will confuse the parser
*/
std::unique_ptr<AstNode> Parser::ParsePrintStmt() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_print));

  bool parenthesis = false;
  if (MatchRead(TokenType::tk_open_parenthesis)) {
    parenthesis = true;
  }

  auto print_ast = std::unique_ptr<AstPrintStmt>(new AstPrintStmt(tok->location, main_module));
  while (true) {
    print_ast->AddArgument(ParseExpression());

    if (!MatchRead(TokenType::tk_comma)) {
      break;
    }
  }

  if (parenthesis) {
    ExpectRead(TokenType::tk_close_parenthesis);
  }

  return std::move(print_ast);
}

std::unique_ptr<AstNode> Parser::ParseReturnStmt() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_return));
  auto expr = ParseExpression();

  return std::unique_ptr<AstReturnStmt>(new AstReturnStmt(tok->location, main_module,
    std::move(expr)));
}

/** For loops work very similarly to how the do in other
    C-style languages. They are equivalent to a while loop,
    only combined with an initial statement and a counter
    which is typically incremented or decremented after the
    end of the block has been reached.

    \todo For each style loops
    \todo For range loops
    \todo Adjust syntax to match new if statements

    For loops are made up of three parts:
      - Initializer
      - Conditional
      - Afterthought
*/
std::unique_ptr<AstNode> Parser::ParseForLoop() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_for));
  ExpectRead(tk_open_parenthesis);

  // read initializer
  std::unique_ptr<AstNode> init_expr = nullptr;
  if (!MatchRead(tk_semicolon)) {
    init_expr = ParseStatement();
    ExpectRead(tk_semicolon);
  }

  // read conditional
  std::unique_ptr<AstNode> cond_expr = nullptr;
  if (MatchRead(tk_semicolon)) {
    cond_expr = std::unique_ptr<AstTrue>(new AstTrue(Location(), main_module));
  } else {
    cond_expr = ParseExpression();
    ExpectRead(tk_semicolon);
  }

  // read increment
  std::unique_ptr<AstNode> inc_expr = nullptr;
  if (!MatchRead(tk_close_parenthesis)) {
    inc_expr = ParseExpression();
    ExpectRead(tk_close_parenthesis);
  }

  std::unique_ptr<AstNode> loop_block = nullptr;
  if (Peek() && Peek()->type == tk_open_brace) {
    loop_block = ParseStatement(); // read the block
  } else {
    auto block_ast = std::unique_ptr<AstBlock>(new AstBlock(Location(), main_module));
    block_ast->children.push_back(ParseStatement());
    loop_block = std::move(block_ast);
  }

  return std::unique_ptr<AstForLoop>(new AstForLoop(tok->location, main_module,
    std::move(init_expr), std::move(cond_expr), std::move(inc_expr), std::move(loop_block)));
}

/** While loops work very similarly to how the do in other
    C-style languages (except for some syntax differences).
    They are simply an 'if' statement, except for the fact that
    they jump back up to the beginning of the loop upon reaching
    the end, but only if the condition is true.

    Example:
      while true:
        print 'infinite loop!', '\n';

      output:
        infinite loop!
        infinite loop!
        infinite loop!
        ...

      var x = 2;
      while x < 6 {
        print 'x = ', x, '\n';
        x+=1;
      }

      output:
        x = 2
        x = 3
        x = 4
        x = 5
*/
std::unique_ptr<AstNode> Parser::ParseWhileLoop() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_while));

  // read conditional
  std::unique_ptr<AstNode> cond_expr = ParseExpression();

  std::unique_ptr<AstNode> loop_block = nullptr;
  if (Peek() && Peek()->type == tk_open_brace) {
    loop_block = ParseStatement(); // read the block
  } else if (MatchRead(tk_colon)) {
    auto block_ast = std::unique_ptr<AstBlock>(new AstBlock(Location(), main_module));
    auto stmt = ParseStatement();
    if (stmt != nullptr) {
      block_ast->children.push_back(std::move(stmt));
    }
    loop_block = std::move(block_ast);
  } else if (MatchRead(tk_semicolon)) {
    // if nothing else, we need a semicolon
    loop_block = std::unique_ptr<AstBlock>(new AstBlock(Location(), main_module));
  } else {
    ErrorMsg(msg_unexpected_token, Location(), Read()->value);
  }

  return std::unique_ptr<AstWhileLoop>(new AstWhileLoop(tok->location, main_module,
    std::move(cond_expr), std::move(loop_block)));
}

std::unique_ptr<AstNode> Parser::ParseTryCatch() {
  Token *tok = ExpectRead(tk_keyword, keyword_tostr(kw_try));

  std::unique_ptr<AstNode> try_block = nullptr;
  if (Peek() && Peek()->type == tk_open_brace) {
    try_block = ParseStatement(); // read the block
  } else if (MatchRead(tk_colon)) {
    auto block_ast = std::unique_ptr<AstBlock>(new AstBlock(Location(), main_module));
    auto stmt = ParseStatement();
    if (stmt != nullptr) {
      block_ast->children.push_back(std::move(stmt));
    }
    try_block = std::move(block_ast);
  } else {
    ErrorMsg(msg_unexpected_token, Location(), Read()->value);
  }

  ExpectRead(tk_keyword, keyword_tostr(kw_catch));

  // catch blocks do not require an identifier for the exception object,
  // they may also be optionally enclosed in parenthesis.
  std::unique_ptr<AstNode> exception_variable(nullptr);

  if (MatchRead(tk_open_parenthesis)) {
    auto *identifier = ExpectRead(tk_identifier);
    if (identifier) {
      exception_variable = std::unique_ptr<AstVariableDeclaration>(new AstVariableDeclaration(Location(), main_module,
        identifier->value, std::unique_ptr<AstNull>(new AstNull(Location(), main_module)), /*make object const*/ true));
    }
    ExpectRead(tk_close_parenthesis);
  } else if (Match(tk_identifier)) {
    auto *identifier = ExpectRead(tk_identifier);
    if (identifier) {
      exception_variable = std::unique_ptr<AstVariableDeclaration>(new AstVariableDeclaration(Location(), main_module,
        identifier->value, std::unique_ptr<AstNull>(new AstNull(Location(), main_module)), /*make object const*/ true));
    }
  }

  std::unique_ptr<AstNode> catch_block = nullptr;
  if (Peek() && Peek()->type == tk_open_brace) {
    catch_block = ParseStatement(); // read the block
  } else if (MatchRead(tk_colon)) {
    auto block_ast = std::unique_ptr<AstBlock>(new AstBlock(Location(), main_module));
    auto stmt = ParseStatement();
    if (stmt != nullptr) {
      block_ast->children.push_back(std::move(stmt));
    }
    catch_block = std::move(block_ast);
  } else {
    ErrorMsg(msg_unexpected_token, Location(), Read()->value);
  }

  return std::unique_ptr<AstTryCatch>(new AstTryCatch(tok->location, main_module,
    std::move(try_block), std::move(catch_block), std::move(exception_variable)));
}
} // namespace avm