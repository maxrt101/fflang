#include "compiler/scanner.h"

#include <cctype>
#include <cstring>
#include <iostream>


Scanner::Scanner(std::string& source) {
  start_ = source.begin();
  current_ = source.begin();
  line_ = 1;
}


bool Scanner::IsAtEnd() const {
  return *current_ == '\0';
}


char Scanner::Advance() {
  current_++;
  return current_[-1];
}


bool Scanner::Match(char expected) {
  if (IsAtEnd()) return false;
  if (*current_ != expected) return false;

  current_++;
  return true;
}


char Scanner::Peek() const {
  if (IsAtEnd()) return '\0';
  return *current_;
}


char Scanner::PeekNext() const {
  return current_[1];
}


void Scanner::SkipWhitespace() {
  for (;;) {
    char c = Peek();
    switch (c) {
      case '\n':
        line_++; [[fallthrough]];
      case ' ':  [[fallthrough]];
      case '\r': [[fallthrough]];
      case '\t':
        Advance();
        break;
      case '/':
        if (PeekNext() == '/') {
          while (Peek() != '\n' && !IsAtEnd()) Advance();
        } else {
          return;
        }
      default:
        return;
    }
  }
}


Token Scanner::MakeToken(TokenType type) const {
  Token token;
  token.type = type;
  token.str = std::string(start_, current_);
  token.line = line_;
  return token;
}


Token Scanner::ErrorToken(const char* msg) const {
  Token token;
  token.type = TOKEN_ERROR;
  token.str = msg;
  token.line = line_;
  return token;
}


Token Scanner::String() {
  while (Peek() != '"' && !IsAtEnd()) {
    if (Peek() == '\n') line_++;
    Advance();
  }

  if (IsAtEnd()) return ErrorToken("Unterminated string");

  Advance(); // Closing double quote
  return MakeToken(TOKEN_STRING);
}


Token Scanner::Number() {
  while (isdigit(Peek())) Advance();

  if (Peek() == '.' && isdigit(PeekNext())) {
    Advance();

    while (isdigit(Peek())) Advance();
  }

  return MakeToken(TOKEN_NUMBER);
}


Token Scanner::Identifier() {
  while (isalpha(Peek()) || isdigit(Peek())) Advance();

  return MakeToken(IdentifierType());
}


TokenType Scanner::IdentifierType() const {
  switch (start_[0]) {
    case 'a': return CheckKeyword(1, 2, "nd", TOKEN_AND);
    case 'b': return CheckKeyword(1, 4, "reak", TOKEN_BREAK);
    case 'c': {
      if (current_ - start_ > 1) {
        switch (start_[1]) {
          case 'l': return CheckKeyword(2, 3, "ass", TOKEN_CLASS);
          case 'o': {
            if (current_ - start_ > 2) {
              switch (start_[2]) {
                case 'n': {
                  if (current_ - start_ > 3) {
                    switch (start_[3]) {
                      case 's': return CheckKeyword(2, 2, "t", TOKEN_CONST);
                      case 't': return CheckKeyword(2, 4, "inue", TOKEN_CONTINUE);
                    }
                  }
                }
              }
            }
          }
        }
      }
      break;
    }
    case 'f': {
      if (current_ - start_ > 1) {
        switch (start_[1]) {
          case 'a': return CheckKeyword(2, 3, "lse", TOKEN_FALSE);
          case 'o': return CheckKeyword(2, 1, "r", TOKEN_FOR);
          case 'n': return TOKEN_FN;
        }
      }
      break;
    }
    case 'e': return CheckKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'i': return CheckKeyword(1, 1, "f", TOKEN_IF);
    case 'n': return CheckKeyword(1, 3, "ull", TOKEN_NULL);
    case 'o': return CheckKeyword(1, 1, "r", TOKEN_OR);
    case 'p': return CheckKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r': return CheckKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's': return CheckKeyword(1, 4, "uper", TOKEN_SUPER);
    case 't': {
      if (current_ - start_ > 1) {
        switch (start_[1]) {
          case 'h': return CheckKeyword(2, 2, "is", TOKEN_THIS);
          case 'r': return CheckKeyword(2, 2, "ue", TOKEN_TRUE);
        }
      }
      break;
    }
    case 'v': return CheckKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w': return CheckKeyword(1, 4, "hile", TOKEN_WHILE);
  }
  return TOKEN_IDENTIFIER;
}


TokenType Scanner::CheckKeyword(int start, int length, const char* rest, TokenType type) const {
  if (current_ - start_ == start + length
   && IterSeqEquBuffer(start_+start, rest, length)) {
    return type;
   }
   return TOKEN_IDENTIFIER;
}


bool Scanner::IterSeqEquBuffer(std::string::iterator itr, const char* buffer, int length) const {
  for (int i = 0; i < length; i++, itr++) {
    if (*itr != buffer[i]) return false;
  }
  return true;
}


Token Scanner::ScanToken() {
  SkipWhitespace();

  start_ = current_;

  if (IsAtEnd()) return MakeToken(TOKEN_EOF);

  char c = Advance();

  if (isalpha(c)) return Identifier();
  if (isdigit(c)) return Number();

  switch (c) {
    case '(': return MakeToken(TOKEN_LEFT_PAREN);
    case ')': return MakeToken(TOKEN_RIGHT_PAREN);
    case '{': return MakeToken(TOKEN_LEFT_BRACE);
    case '}': return MakeToken(TOKEN_RIGHT_BRACE);
    case ';': return MakeToken(TOKEN_SEMICOLON);
    case ',': return MakeToken(TOKEN_COMMA);
    case '.': return MakeToken(TOKEN_DOT);
    case '-': return MakeToken(TOKEN_MINUS);
    case '+': return MakeToken(TOKEN_PLUS);
    case '/': return MakeToken(TOKEN_SLASH);
    case '*': return MakeToken(TOKEN_STAR);
    case '!': return MakeToken(Match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=': return MakeToken(Match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<': return MakeToken(Match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>': return MakeToken(Match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"': return String();
  }

  return ErrorToken("Unexpected token.");
}

