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

static Chunk* CurrentChunk() {
  return &current_state->function->chunk;
}


/** TODO: make const */
ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {&Compiler::Grouping, &Compiler::Call,    PREC_CALL},
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
  [TOKEN_AND]           = {NULL,                &Compiler::And,     PREC_AND},
  [TOKEN_CLASS]         = {NULL,                NULL,               PREC_NONE},
  [TOKEN_ELSE]          = {NULL,                NULL,               PREC_NONE},
  [TOKEN_FALSE]         = {&Compiler::Literal,  NULL,               PREC_NONE},
  [TOKEN_FOR]           = {NULL,                NULL,               PREC_NONE},
  [TOKEN_FN]            = {NULL,                NULL,               PREC_NONE}, // &Compiler::Lambda, <PREC_CALL
  [TOKEN_IF]            = {NULL,                NULL,               PREC_NONE},
  [TOKEN_NULL]          = {&Compiler::Literal,  NULL,               PREC_NONE},
  [TOKEN_OR]            = {NULL,                &Compiler::Or,      PREC_OR},
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


CompilerState::CompilerState(FunctionType type, std::string name)
    : local_count(0), scope_depth(0), type(type) {
  function = ObjFunction::New();

  if (type != TYPE_SCRIPT) {
    function->name = ObjString::FromStr(name);
  }

  Local* local = &locals[local_count++];
  local->depth = 0;
  local->name.str = "";

  enclosing = current_state;
  current_state = this;
}


ObjFunction* CompilerState::End(Compiler* compiler) {
  compiler->EndCompilation();
#ifdef _DEBUG_DUMP_COMPILED
  if (compiler->HadError())
    debug::DisassembleChunk(*CurrentChunk(), function->name ? function->name->str : "<script>");
#endif
  current_state = current_state->enclosing;
  return function;
}


Compiler::Compiler(std::string& source) : scanner_(source) {}


bool Compiler::HadError() const {
  return had_error_;
}


void Compiler::EndCompilation() {
  EmitReturn();
}


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
  CurrentChunk()->AppendCode(byte, previous_.line);
}


void Compiler::EmitBytes(uint8_t byte1, uint8_t byte2) {
  EmitByte(byte1);
  EmitByte(byte2);
}


void Compiler::EmitReturn() {
  EmitByte(OP_NULL);
  EmitByte(OP_RETURN);
}


int Compiler::EmitJump(uint8_t op) {
  EmitByte(op);
  EmitBytes(0xff, 0xff);
  return CurrentChunk()->code.size() - 2;
}


void Compiler::PatchJump(int offset) {
  int jump = CurrentChunk()->code.size() - offset - 2;

  abi::NumericData data;
  
  if (jump > UINT16_MAX) {
    Error("Too much code to jump over.");
  }

  data.u16[0] = jump;

  CurrentChunk()->code[offset] = data.u8[0]; //(jump >> 8) & 0xff;
  CurrentChunk()->code[offset+1] = data.u8[1]; //jump & 0xff;
}


void Compiler::EmitLoop(int loop_start) {
  EmitByte(OP_LOOP);

  int offset = CurrentChunk()->code.size() - loop_start + 2;
  if (offset > UINT16_MAX) {
    Error("Loop body too large");
  }

  abi::NumericData jump;
  jump.u16[0] = offset;

  EmitByte(jump.u8[0]);
  EmitByte(jump.u8[1]);
}


void Compiler::PatchRemoteJump(int loop, int offset) {
  if (offset > UINT16_MAX) {
    Error("Too much code to jump over.");
  }

  abi::NumericData data;
  data.u16[0] = offset;
  CurrentChunk()->code[loop] = data.u8[0];
  CurrentChunk()->code[loop+1] = data.u8[1];
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
  int constant = CurrentChunk()->AddConstant(value);
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
  ObjString* str = ObjString::FromStr(name->str);
  return MakeConstant(str->AsValue());
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
  if (current_state->scope_depth == 0) return;
  current_state->locals[current_state->local_count - 1].depth = current_state->scope_depth;
}


uint8_t Compiler::ArgumentList() {
  uint8_t arg_count = 0;
  if (!Check(TOKEN_RIGHT_PAREN)) {
    do {
      Expression();
      if (arg_count == 255) {
        Error("Can't have more than 255 arguments.");
      }
      arg_count++;
    } while (Match(TOKEN_COMMA));
  }

  Consume(TOKEN_RIGHT_PAREN, "Expected ')' after function arguments.");
  return arg_count;
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


void Compiler::BeginLoop() {
  loops_.push_back(LoopRecord());
}


void Compiler::EndLoop() {
  loops_.pop_back();
}


LoopRecord& Compiler::GetLoop(int nest) {
  return loops_[loops_.size()-1 - nest];
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
  } else if (Match(TOKEN_FN)) {
    FnDeclaration();
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


void Compiler::FnDeclaration() {
  int global = ParseVariable("Expected function name.");
  MarkInitialized();
  Function(TYPE_FUNCTION);
  DefineVariable(global, true);
}


void Compiler::Statement() {
  if (Match(TOKEN_LEFT_BRACE)) {
    BeginScope();
    Block();
    EndScope();
  } else if (Match(TOKEN_IF)) {
    IfStatement();
  } else if (Match(TOKEN_WHILE)) {
    WhileStatement();
  } else if (Match(TOKEN_FOR)) {
    ForStatement();
  } else if (Match(TOKEN_BREAK)) {
    BreakStatement();
  } else if (Match(TOKEN_CONTINUE)) {
    ContinueStatement();
  } else if (Match(TOKEN_RETURN)) {
    ReturnStatement();
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


void Compiler::WhileStatement() {
  int loop_start = CurrentChunk()->code.size();

  Consume(TOKEN_LEFT_PAREN, "Expected '(' after 'while'.");
  Expression();
  Consume(TOKEN_RIGHT_PAREN, "Expexted ')' after condition.");

  int exit_jump = EmitJump(OP_JUMP_IF_FALSE);

  BeginLoop();
  
  EmitByte(OP_POP);
  Statement();

  EmitLoop(loop_start);

  PatchJump(exit_jump);
  EmitByte(OP_POP);

  // Patch all 'continues'
  for (int continue_jump : GetLoop().start_jump) {
    PatchRemoteJump(continue_jump, loop_start);
  }

  // Patch all 'breaks'
  for (int break_jump : GetLoop().end_jump) {
    PatchJump(break_jump);
  }

  EndLoop();
}


void Compiler::ForStatement() {
  BeginScope();

  Consume(TOKEN_LEFT_PAREN, "Expected '(' after 'for'.");
  
  // Consume(TOKEN_SEMICOLON, "Expected ';' after 'for' initializer.");
  if (Match(TOKEN_SEMICOLON)) {
    // No initializer
  } else if (Match(TOKEN_VAR)) {
    VarDeclaration(true);
  } else {
    ExpressionStatement();
  }

  int loop_start = CurrentChunk()->code.size();

  int exit_jump = -1;
  if (!Match(TOKEN_SEMICOLON)) {
    Expression();
    Consume(TOKEN_SEMICOLON, "Expected ';' after 'for' condition.");
  
    exit_jump = EmitJump(OP_JUMP_IF_FALSE);
    EmitByte(OP_POP); // Condition
  }

  if (!Match(TOKEN_RIGHT_PAREN)) {
    int body_jump = EmitJump(OP_JUMP);
    
    int increment_start = CurrentChunk()->code.size();
    Expression();
    EmitByte(OP_POP);
    Consume(TOKEN_RIGHT_PAREN, "Expected ')' after 'for' increment.");
  
    EmitLoop(loop_start);
    loop_start = increment_start;
    PatchJump(body_jump);
  }

  BeginLoop();
  Statement();

  EmitLoop(loop_start);

  if (exit_jump != -1) {
    PatchJump(exit_jump);
    EmitJump(OP_POP); // Condition
  }

  // Patch all 'continues'
  for (int continue_jump : GetLoop().start_jump) {
    PatchRemoteJump(continue_jump, loop_start);
  }

  // Patch all 'breaks'
  for (int break_jump : GetLoop().end_jump) {
    PatchJump(break_jump);
  }
  
  EndLoop();
  EndScope();
}


void Compiler::BreakStatement() {
  // TODO: check for Number for nested break
  Consume(TOKEN_SEMICOLON, "Expected ';' after expression.");
  int jump = EmitJump(OP_JUMP);
  GetLoop().end_jump.push_back(jump);
}


void Compiler::ContinueStatement() {
  Consume(TOKEN_SEMICOLON, "Expected ';' after expression.");
  int jump = EmitJump(OP_LOOP);
  GetLoop().start_jump.push_back(jump);
}


void Compiler::ReturnStatement() {
  if (current_state->type == TYPE_SCRIPT) {
    Error("Can't return from top-level code.");
  }
  if (Match(TOKEN_SEMICOLON)) {
    EmitReturn();
  } else {
    Expression();
    Consume(TOKEN_SEMICOLON, "Expected ';' after return value.");
    EmitByte(OP_RETURN);
  }
}


void Compiler::Function(FunctionType type) {
  CompilerState f_state(type, previous_.str); // function_state
  
  BeginScope();

  Consume(TOKEN_LEFT_PAREN, "Expected '(' after function name.");
  if (!Check(TOKEN_RIGHT_PAREN)) {
    do {
      current_state->function->arity++;
      if (current_state->function->arity > 255) {
        ErrorAtCurrent("Can't have more than 255 parameters.");
      }

      int param_constant = ParseVariable("Expected parameter name.");
      DefineVariable(param_constant, true);
    } while (Match(TOKEN_COMMA));
  }
  Consume(TOKEN_RIGHT_PAREN, "Expected ')' after parameter declaration.");

  Consume(TOKEN_LEFT_BRACE, "Expected '{' before function body.");
  Block();

  ObjFunction* function = f_state.End(this);
  int constant = MakeConstant(function->AsValue());
  EmitCheckLong(constant, OP_CONSTANT, OP_CONSTANT_LONG);
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


void Compiler::And(bool can_assign) {
  int end_jump = EmitJump(OP_JUMP_IF_FALSE);

  EmitByte(OP_POP);
  ParsePrecedence(PREC_AND);

  PatchJump(end_jump);
}


void Compiler::Or(bool can_assign) {
  int else_jump = EmitJump(OP_JUMP_IF_FALSE);
  int end_jump = EmitJump(OP_JUMP);

  PatchJump(else_jump);
  EmitByte(OP_POP);

  ParsePrecedence(PREC_OR);
  PatchJump(end_jump);
}


void Compiler::Call(bool can_assign) {
  uint8_t arg_count = ArgumentList();
  EmitBytes(OP_CALL, arg_count);
}


ObjFunction* Compiler::Compile() {
  had_error_ = false;
  panic_mode_ = false;

  CompilerState c_state(TYPE_SCRIPT, ""); // compiler state

  Advance();
  while (!Match(TOKEN_EOF)) {
    Declaration();
  }

  ObjFunction* function = c_state.End(this);

  return HadError() ? nullptr : function;
}

