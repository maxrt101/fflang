#include "compiler/compiler.h"
#include "core/value.h"
#include "core/object.h"

#include "utils/abi.h"
#include "utils/math.h"

#include "debug/disasm.h"

#include <cstdio>
#include <iostream>

static constexpr size_t kMaxLongConstant = math::ConstexprPow(2, 32);


static CompilerState* current_state = nullptr;


/** TODO: make const */
ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {&Compiler::Grouping, NULL,               PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,                NULL,               PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,                NULL,               PREC_NONE},
  [TOKEN_RIGHT_BRACE]   = {NULL,                NULL,               PREC_NONE},
  [TOKEN_COMMA]         = {NULL,                NULL,               PREC_NONE},
  [TOKEN_DOT]           = {NULL,                NULL,               PREC_NONE},
  [TOKEN_MINUS]         = {&Compiler::Unary,    &Compiler::Binary,  PREC_TERM},
  [TOKEN_PLUS]          = {NULL,                &Compiler::Binary,  PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,                NULL,               PREC_NONE},
  [TOKEN_SLASH]         = {NULL,                &Compiler::Binary,  PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,                &Compiler::Binary,  PREC_FACTOR},
  [TOKEN_BANG]          = {&Compiler::Unary,    NULL,               PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,                &Compiler::Binary,  PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,                NULL,               PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,                &Compiler::Binary,  PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,                &Compiler::Binary,  PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,                &Compiler::Binary,  PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,                &Compiler::Binary,  PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,                &Compiler::Binary,  PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {&Compiler::Variable, NULL,               PREC_NONE},
  [TOKEN_STRING]        = {&Compiler::String,   NULL,               PREC_NONE},
  [TOKEN_NUMBER]        = {&Compiler::Number,   NULL,               PREC_NONE},
  [TOKEN_AND]           = {NULL,                NULL,               PREC_NONE},
  [TOKEN_CLASS]         = {NULL,                NULL,               PREC_NONE},
  [TOKEN_ELSE]          = {NULL,                NULL,               PREC_NONE},
  [TOKEN_FALSE]         = {&Compiler::Literal,  NULL,               PREC_NONE},
  [TOKEN_FOR]           = {NULL,                NULL,               PREC_NONE},
  [TOKEN_FN]            = {NULL,                NULL,               PREC_NONE},
  [TOKEN_IF]            = {NULL,                NULL,               PREC_NONE},
  [TOKEN_NULL]          = {&Compiler::Literal,  NULL,               PREC_NONE},
  [TOKEN_OR]            = {NULL,                NULL,               PREC_NONE},
  [TOKEN_PRINT]         = {NULL,                NULL,               PREC_NONE},
  [TOKEN_RETURN]        = {NULL,                NULL,               PREC_NONE},
  [TOKEN_SUPER]         = {NULL,                NULL,               PREC_NONE},
  [TOKEN_THIS]          = {NULL,                NULL,               PREC_NONE},
  [TOKEN_TRUE]          = {&Compiler::Literal,  NULL,               PREC_NONE},
  [TOKEN_VAR]           = {NULL,                NULL,               PREC_NONE},
  [TOKEN_WHILE]         = {NULL,                NULL,               PREC_NONE},
  [TOKEN_ERROR]         = {NULL,                NULL,               PREC_NONE},
  [TOKEN_EOF]           = {NULL,                NULL,               PREC_NONE},
};


CompilerState::CompilerState() : local_count(0), scope_depth(0) {
  current_state = this;
}


Compiler::Compiler(std::string& source) : scanner_(source) {}


void Compiler::Advance() {
  previous_ = current_;

  for (;;) {
    current_ = scanner_.ScanToken();
    if (current_.type != TOKEN_ERROR) break;

    ErrorAtCurrent(std::string(current_.str));
  }
}


bool Compiler::Check(TokenType type) const {
  return current_.type == type;
}


bool Compiler::Match(TokenType type) {
  if (!Check(type)) return false;
  Advance();
  return true;
}


void Compiler::Consume(TokenType type, std::string msg) {
  if (current_.type == type) {
    Advance();
    return;
  }

  ErrorAtCurrent(msg);
}


void Compiler::Error(std::string msg) {
  ErrorAt(previous_, msg);
}


void Compiler::ErrorAtCurrent(std::string msg) {
  ErrorAt(current_, msg);
}


void Compiler::ErrorAt(Token& token, std::string msg) {
  if (panic_mode_) return;
  panic_mode_ = true;
  fprintf(stderr, "[line %d] Error", token.line);

  if (token.type == TOKEN_EOF) {
    fprintf(stderr, " at the end");
  } else {
    fprintf(stderr, " at '%.*s'", (int)token.str.size(), token.str.data());
  }

  fprintf(stderr, ": %s\n", msg.c_str());
  had_error_ = true;
}


void Compiler::Syncronize() {
  panic_mode_ = false;

  while (current_.type != TOKEN_EOF) {
    if (previous_.type == TOKEN_SEMICOLON) return;

    switch (current_.type) {
      case TOKEN_CLASS:
      case TOKEN_FN:
      case TOKEN_VAR:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
        return;
      default:
        ; /* Do Nothing */
    }

    Advance();
  }
}


void Compiler::EmitByte(uint8_t byte) {
  current_chunk_->AppendCode(byte, previous_.line);
}


void Compiler::EmitBytes(uint8_t byte1, uint8_t byte2) {
  EmitByte(byte1);
  EmitByte(byte2);
}


void Compiler::EmitReturn() {
  EmitByte(OP_RETURN);
}


int Compiler::EmitJump(uint8_t op) {
  EmitByte(op);
  EmitBytes(0xff, 0xff);
  return current_chunk_->code.size() - 2;
}


void Compiler::PatchJump(int offset) {
  int jump = current_chunk_->code.size() - offset - 2;

  abi::NumericData data;
  
  if (jump > UINT16_MAX) {
    Error("Too much code to jump over.");
  }

  data.u16[0] = jump;

  current_chunk_->code[offset] = data.u8[0]; //(jump >> 8) & 0xff;
  current_chunk_->code[offset+1] = data.u8[1]; //jump & 0xff;
}


void Compiler::EmitCheckLong(int val, uint8_t op, uint8_t long_op) {
  if (val > UINT8_MAX) {
    abi::NumericData tmp;
    tmp.i32 = val;
    EmitByte(long_op);
    EmitByte(tmp.u8[0]);
    EmitByte(tmp.u8[1]);
    EmitByte(tmp.u8[2]);
    EmitByte(tmp.u8[3]);
  } else {
    EmitBytes(op, val);
  }
}


void Compiler::EmitConstant(Value value) {
  int constant = MakeConstant(value);
  EmitCheckLong(constant, OP_CONSTANT, OP_CONSTANT_LONG);
}


int Compiler::MakeConstant(Value value) {
  int constant = current_chunk_->AddConstant(value);
  if (constant > kMaxLongConstant) {
      Error("Too many constant in one chunk.");
      return 0;
  }
  return constant;
}


int Compiler::ParseVariable(const char* err_msg) {
  Consume(TOKEN_IDENTIFIER, err_msg);

  DeclareVariable();
  if (current_state->scope_depth > 0) return 0;

  return IdentifierConstant(&previous_);
}


int Compiler::IdentifierConstant(Token* name) {
  return MakeConstant(ObjString::FromStr(name->str)->AsValue());
}


void Compiler::DefineVariable(int global, bool assignable) {
  if (current_state->scope_depth > 0) {
    MarkInitialized();
    if (!assignable) EmitByte(OP_MAKECONST);
    return;
  }

  if (!assignable) EmitByte(OP_MAKECONST);
  EmitCheckLong(global, OP_DEFINE_GLOBAL, OP_DEFINE_GLOBAL_LONG);
}


void Compiler::DeclareVariable() {
  if (current_state->scope_depth == 0) return;

  Token* name = &previous_;
  
  for (int i = current_state->local_count - 1; i >= 0; i--) {
    Local* local = &current_state->locals[i];
    if (local->depth != -1 && local->depth < current_state->scope_depth) {
      break;
    }

    if (name->str == local->name.str) {
      Error("Redifinition of variable '" + name->str + "' in the same scope.");
    }
  }

  AddLocal(*name);
}


void Compiler::NamedVariable(Token name, bool can_assign) {
  int arg = ResolveLocal(current_state, &name);

  if (arg != -1) {
    if (can_assign && Match(TOKEN_EQUAL)) {
      Expression();
      EmitBytes(OP_SET_LOCAL, arg);
    } else {
      EmitBytes(OP_GET_LOCAL, arg);
    }
  } else {
    arg = IdentifierConstant(&name);
    if (can_assign && Match(TOKEN_EQUAL)) {
      Expression();
      EmitCheckLong(arg, OP_SET_GLOBAL, OP_SET_GLOBAL_LONG);
    } else {
      EmitCheckLong(arg, OP_GET_GLOBAL, OP_GET_GLOBAL_LONG);
    }
  }
}


void Compiler::AddLocal(Token name) {
  if (current_state->local_count >= kLocalsSize) {
    Error("Too many local variables in one scope,");
    return;
  }
  Local* local = &current_state->locals[current_state->local_count++];
  local->name = name;
  local->depth = -1;
}


int Compiler::ResolveLocal(CompilerState* state, Token* name) {
  for (int i = state->local_count - 1; i >= 0; i--) {
    Local* local = &state->locals[i];
    if (name->str == local->name.str) {
      if (local->depth == -1) {
        Error("Self reference to uninitialized variable inside it's initializer.");
      }
      return i;
    }
  }

  return -1;
}


void Compiler::MarkInitialized() {
  current_state->locals[current_state->local_count - 1].depth = current_state->scope_depth;
}


void Compiler::EndCompiling() {
  EmitReturn();
}


void Compiler::BeginScope() {
  current_state->scope_depth++;
}


void Compiler::EndScope() {
  current_state->scope_depth--;

  while (current_state->local_count > 0
      && current_state->locals[current_state->local_count - 1].depth > current_state->scope_depth) {
    EmitByte(OP_POP);
    current_state->local_count--;
  }
}


void Compiler::ParsePrecedence(Precedence precedence) {
  Advance();
  ParseFn prefix_rule = GetRule(previous_.type)->prefix;
  if (prefix_rule == NULL) {
    Error("Expected expression.");
    return;
  }

  bool can_assign = precedence <= PREC_ASSIGNMENT;
  (this->*(prefix_rule))(can_assign);

  while (precedence <= GetRule(current_.type)->precedence) {
    Advance();
    ParseFn infix_rule = GetRule(previous_.type)->infix;
    (this->*(infix_rule))(can_assign);
  }

  if (can_assign && Match(TOKEN_EQUAL)) {
    Error("Invalid assignment taeget.");
  }
}


ParseRule* Compiler::GetRule(TokenType type) {
  return &rules[type];
}


void Compiler::Declaration() {
  if (Match(TOKEN_VAR)) {
    VarDeclaration(true);
  } else if (Match(TOKEN_CONST)) {
    VarDeclaration(false);
  } else {
    Statement();
  }

  if (panic_mode_) Syncronize();
}


void Compiler::VarDeclaration(bool assignable) {
  int global = ParseVariable("Expected variable name.");

  if (Match(TOKEN_EQUAL)) {
    Expression();
  } else {
    EmitByte(OP_NULL);
  }

  Consume(TOKEN_SEMICOLON, "Expected ';' after variable declaration.");
  DefineVariable(global, assignable);
}


void Compiler::Statement() {
  if (Match(TOKEN_LEFT_BRACE)) {
    BeginScope();
    Block();
    EndScope();
  } else if (Match(TOKEN_IF)) {
    IfStatement();
  } else if (Match(TOKEN_PRINT)) {
    PrintStatement();
  } else {
    ExpressionStatement();
  }
}


void Compiler::ExpressionStatement() {
  Expression();
  Consume(TOKEN_SEMICOLON, "Expected ';' after expression.");
  EmitByte(OP_POP);
}


void Compiler::PrintStatement() {
  Expression();
  Consume(TOKEN_SEMICOLON, "Expected ';' after expression.");
  EmitByte(OP_PRINT);
}


void Compiler::IfStatement() {
  Consume(TOKEN_LEFT_PAREN, "Expected '(' after 'if'.");
  Expression();
  Consume(TOKEN_RIGHT_PAREN, "Expected ')' after 'if'.");

  int then_jump = EmitJump(OP_JUMP_IF_FALSE);
  EmitByte(OP_POP);
  Statement();

  int else_jump = EmitJump(OP_JUMP);

  PatchJump(then_jump);
  EmitByte(OP_POP);

  if (Match(TOKEN_ELSE)) {
    Statement();
  }
  PatchJump(else_jump);
}


void Compiler::Block() {
  while (!Check(TOKEN_RIGHT_BRACE) && !Check(TOKEN_EOF)) {
    Declaration();
  }

  Consume(TOKEN_RIGHT_BRACE, "Expected '}' after block.");
}


void Compiler::Expression() {
  ParsePrecedence(PREC_ASSIGNMENT);
}


void Compiler::Grouping(bool can_assign) {
  Expression();
  Consume(TOKEN_RIGHT_PAREN, "Exprected ')' after expression.");
}


void Compiler::Unary(bool can_assign) {
  TokenType operator_type = previous_.type;

  ParsePrecedence(PREC_UNARY);

  switch (operator_type) {
    case TOKEN_BANG:  EmitByte(OP_NOT); break;
    case TOKEN_MINUS: EmitByte(OP_NEGATE); break;
    default:
      return;
  }
}


void Compiler::Binary(bool can_assign) {
  TokenType operator_type = previous_.type;

  ParseRule* rule = GetRule(operator_type);
  ParsePrecedence((Precedence)(rule->precedence + 1));

  switch (operator_type) { // TODO, when objects are implemented, use some kind of operator methods
    case TOKEN_BANG_EQUAL:    EmitBytes(OP_EQUAL, OP_NOT); break;
    case TOKEN_EQUAL_EQUAL:   EmitByte(OP_EQUAL); break;
    case TOKEN_GREATER:       EmitByte(OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: EmitBytes(OP_LESS, OP_NOT); break;
    case TOKEN_LESS:          EmitByte(OP_LESS); break;
    case TOKEN_LESS_EQUAL:    EmitBytes(OP_GREATER, OP_NOT); break;
    case TOKEN_PLUS:          EmitByte(OP_ADD); break;
    case TOKEN_MINUS:         EmitByte(OP_SUBTRACT); break;
    case TOKEN_STAR:          EmitByte(OP_MULTIPLY); break;
    case TOKEN_SLASH:         EmitByte(OP_DIVIDE); break;
    default:
      return;
  }
}


void Compiler::Number(bool can_assign) {
  double value = std::stod(previous_.str);
  EmitConstant(Value(value));
}


void Compiler::Literal(bool can_assign) {
  switch (previous_.type) {
    case TOKEN_FALSE: EmitByte(OP_FALSE); break;
    case TOKEN_NULL:  EmitByte(OP_NULL);  break;
    case TOKEN_TRUE:  EmitByte(OP_TRUE);  break;
    default:
      return;
  }
}


void Compiler::String(bool can_assign) {
  std::string s = previous_.str;
  s.erase(s.begin());
  s.pop_back();
  EmitConstant(Value(ObjString::FromStr(s)->AsObj()));
}


void Compiler::Variable(bool can_assign) {
  NamedVariable(previous_, can_assign);
}


bool Compiler::Compile(Chunk* chunk) {
  had_error_ = false;
  panic_mode_ = false;

  CompilerState c_state;
  current_chunk_ = chunk;

  Advance();
  while (!Match(TOKEN_EOF)) {
    Declaration();
  }
  EndCompiling();

#ifdef _DEBUG_DUMP_COMPILED
  if (!had_error_)
    debug::DisassembleChunk(*chunk, "code");
#endif

  return !had_error_;
}

