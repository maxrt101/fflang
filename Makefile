# fflang (Now in C++!)

CXX       := clang++
CXXFLAGS  := -std=c++17 -Isrc/
DBGFLAGS  := -g -D_DEBUG -D_DEBUG_EXECUTION_TRACING -D_DEBUG_TRACE_STACK -D_DEBUG_DUMP_COMPILED
LIBS      := -lreadline
SRC       := $(wildcard src/*.cc) $(wildcard src/*/*.cc)
OUT       := ff

.PHONY: all

all:
	$(CXX) $(CXXFLAGS) $(LIBS) $(SRC) -o $(OUT)

debug:
	$(CXX) $(CXXFLAGS) $(DBGFLAGS) $(LIBS) $(SRC) -o $(OUT)

