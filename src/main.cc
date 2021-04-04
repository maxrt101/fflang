#include "core/vm.h"
#include "utils/die.h"
#include "version.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <string>

#include <readline/readline.h>
#include <readline/history.h>

constexpr auto kReplPrompt = "> ";

static void Repl() {
  VM vm;
  SetCurrent(&vm);
  char* buffer = NULL;
  printf("FF v%s\n", kVersionString);

  for (;;) {
    buffer = readline(kReplPrompt);
    if (buffer == NULL) break;
    if (strlen(buffer) > 0) {
      std::string input(buffer);
      add_history(buffer);
      vm.Interpret(input);
    }
    free(buffer);
  }
}

static void RunFile(std::string filename) {
  std::ifstream file(filename);
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source = buffer.str();
  VM vm;
  SetCurrent(&vm);

  InterpretResult result = vm.Interpret(source);
  if (result == InterpretResult::kCompileError) die(65);
  if (result == InterpretResult::kRuntimeError) die(70);
}

int main(int argc, char ** argv) {
  if (argc == 1) {
    Repl();
  } else if (argc == 2) {
    RunFile(argv[1]);
  } else {
    fprintf(stderr, "Usage: %s [FILE]\n", argv[0]);
    die();
  }

  return 0;
}

