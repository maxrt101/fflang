#ifndef FF_COMPILER_COMPILER_H_
#define FF_COMPILER_COMPILER_H_

#include <vector>
#include <string>

#include "core/chunk.h"
#include "core/object.h"
#include "core/config.h"
#include "compiler/scanner.h"

enum Precedence{
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_LAMBDA,      // fn() {}
  PREC_PRIMARY
};

struct ParseRule;
struct CompilerState;

struct Local {
  Token name;
  int depth;
};

enum FunctionType {
  TYPE_FUNCTION,
  TYPE_LAMBDA,
  TYPE_SCRIPT,
};

struct LoopRecord {
  std::vector<int> start_jump;
  std::vector<int> end_jump;
};


class Compiler {
 private:
  Scanner scanner_;
  Token current_;
  Token previous_;
  bool had_error_;
  bool panic_mode_;
  std::vector<LoopRecord> loops_;

 public:
  Compiler(std::string& source);
  ObjFunction* Compile();
  bool HadError() const;
  void EndCompilation(bool emit_null_return = true);

 private:
  void Advance();
  bool Check(TokenType type) const;
  bool Match(TokenType type);
  void Consume(TokenType type, std::string msg);

  void Error(std::string msg);
  void ErrorAtCurrent(std::string msg);
  void ErrorAt(Token& token, std::string msg);
  void Syncronize();

  void EmitByte(uint8_t byte);
  void EmitBytes(uint8_t byte1, uint8_t byte2);
  void EmitReturn();
  int  EmitJump(uint8_t op);
  void PatchJump(int offset);
  void EmitLoop(int loop_start);
  void PatchRemoteJump(int loop, int offset);

  void EmitCheckLong(int val, uint8_t op, uint8_t long_op);
  void EmitConstant(Value value);
  int  MakeConstant(Value value);
  
  int  ParseVariable(const char* err_msg);
  int  IdentifierConstant(Token* name);
  void DefineVariable(int global, bool assignable);
  void DeclareVariable();
  void NamedVariable(Token name, bool can_assign);
  void AddLocal(Token name);
  int  ResolveLocal(CompilerState* state, Token* name);
  void MarkInitialized();
  uint8_t ArgumentList();

  void BeginScope();
  void EndScope();

  void BeginLoop();
  void EndLoop();
  LoopRecord& GetLoop(int nest = 0);
  
  void ParsePrecedence(Precedence precedence);
  ParseRule* GetRule(TokenType type);

 public: // for the rule table to work
  void Grouping(bool can_assign);
  void Unary(bool can_assign);
  void Binary(bool can_assign);
  void Number(bool can_assign);
  void Literal(bool can_assign);
  void String(bool can_assign);
  void Variable(bool can_assign);
  void And(bool can_assign);
  void Or(bool can_assign);
  void Call(bool can_assign);
  void Lambda(bool can_assign);
 
 private:
  void Declaration();
  void Statement();
  void Expression();
  void VarDeclaration(bool assignable);
  void FnDeclaration();
  void ExpressionStatement();
  void PrintStatement();
  void Block();
  void IfStatement();
  void WhileStatement();
  void ForStatement();
  void BreakStatement();
  void ContinueStatement();
  void ReturnStatement();

  void Function(FunctionType type);
};


typedef void (Compiler::*ParseFn)(bool);


struct ParseRule {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};


struct CompilerState {
  CompilerState* enclosing;

  ObjFunction* function;
  FunctionType type;

  Local locals[kLocalsSize];
  int local_count;
  int scope_depth;

 public:
  CompilerState(FunctionType type, std::string name);
  ObjFunction* End(Compiler* compiler, bool emit_null_return = true);
};

#endif

