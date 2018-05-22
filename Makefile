src = $(wildcard *.cpp)

CXX = g++
CFLAGS = -Wall -O2 -std=c++11
LDFLAGS = -lpthread

all: prog

prog: $(src)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

check: prog
	valgrind ./prog

.PHONY: all clean
clean:
	rm -f  prog
