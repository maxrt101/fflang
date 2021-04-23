#ifndef FF_COMPILER_SCANNER_H_
#define FF_COMPILER_SCANNER_H_

#include <cstdint>
#include <string>
#include <string_view>


enum TokenType : uint8_t {
  // Single-character tokens.
  TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
  TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
  TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,

  // One or two character tokens.
  TOKEN_BANG, TOKEN_BANG_EQUAL,
  TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER, TOKEN_GREATER_EQUAL,
  TOKEN_LESS, TOKEN_LESS_EQUAL,
  TOKEN_LEFT_ARROW, TOKEN_RIGHT_ARROW,

  // Literals.
  TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

  // Keywords.
  TOKEN_AND, TOKEN_BREAK, TOKEN_CLASS, TOKEN_CONTINUE,
  TOKEN_CONST, TOKEN_ELSE, TOKEN_FALSE,
  TOKEN_FOR, TOKEN_FN, TOKEN_IF, TOKEN_NULL, TOKEN_OR,
  TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
  TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

  TOKEN_ERROR,
  TOKEN_EOF
};


struct Token {
  TokenType type;
  std::string str;
  int line;
};


class Scanner {
 private:
  std::string::iterator start_;
  std::string::iterator current_;
  int line_;

 public:
  Scanner(std::string& source);

  Token ScanToken();
 
 private:
  bool IsAtEnd() const;
  char Advance();
  bool Match(char expected);
  char Peek() const;
  char PeekNext() const;
  void SkipWhitespace();

  Token MakeToken(TokenType type) const;
  Token ErrorToken(const char* msg) const;

  Token String();
  Token Number();
  Token Identifier();
  TokenType IdentifierType() const;
  TokenType CheckKeyword(int start, int length, const char* rest, TokenType type) const;
  bool IterSeqEquBuffer(std::string::iterator itr, const char* buffer, int length) const;
};

#endif

