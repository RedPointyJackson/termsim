CPPFLAGS=-g -O2 -std=c++11 -pedantic -Wall -Wextra -Wno-sign-compare
LDDFLAGS=-lpthread -lncursesw
CXX=clang++
CC=clang

OBJECTS = Tui.o MsgBox.o ScatterBox.o

all: termsim nbody

termsim: termsim.cpp ${OBJECTS}
	$(CXX) $(CPPFLAGS) $(LDDFLAGS) ${OBJECTS} termsim.cpp -o termsim

Tui.o: Tui.cpp Tui.hpp
MsgBox.o: MsgBox.cpp MsgBox.hpp
ScatterBox.o: ScatterBox.cpp ScatterBox.hpp

.PHONY = nbody clean

ising:
	$(CC) -lm tests/ising.c -o tests/ising

nbody:
	$(CC) -lm tests/nbody.c -o tests/nbody

clean:
	rm -f termsim ${OBJECTS} tests/nbody
