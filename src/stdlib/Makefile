# stdlib

CXX       		:= clang++
CXXFLAGS  		:= -std=c++17 -I../ -g
LDFLAGS				:= -L../../ -lff
OBJS					:=  $(addsuffix .o, $(basename $(wildcard *.cc)))

.PHONY: all

all: compile clean

compile: $(OBJS)

$(OBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) -c -fPIC $< -o $@
	$(CXX) $(CXXFLAGS) -shared $(LDFLAGS) $@ -o $(addsuffix .so, $(basename $@))

clean:
	rm $(OBJS) $(DEPENDENCIES)