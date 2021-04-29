# fflang (Now in C++!)

CXX       := clang++
AR       	:= ar
CXXFLAGS  := -std=c++17 -Isrc/
DBGFLAGS  := -g -D_DEBUG -D_DEBUG_EXECUTION_TRACING -D_DEBUG_TRACE_STACK -D_DEBUG_DUMP_COMPILED
LDFLAGS  	:= -lreadline -ldl -L. -lff
OBJS      := src/compiler/scanner.o src/compiler/compiler.o src/utils/shared_lib.o src/debug/disasm.o src/core/api.o src/core/chunk.o src/core/memory.o src/core/module.o src/core/object.o src/core/value.o src/core/vm.o
NAME      := ff
LIBNAME		:= lib$(NAME).a

.PHONY: all

all: compile clean
	$(CXX) $(CXXFLAGS) $(LDFLAGS) src/main.cc -o $(NAME)

compile: $(OBJS)
	$(AR) cr $(LIBNAME) $(OBJS)

$(OBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

#debug:
#	$(CXX) $(CXXFLAGS) $(DBGFLAGS) $(LDFLAGS) $(SRC) -o $(OUT)

clean:
#	rm $(OBJS)
