#
# CS 11 C++ Advanced track: Makefile for lab 4
#

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Werror -O3

OBJS = bbrot.o mbrot.o

all: bbrot

bbrot: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

test: bbrot
	./bbrot > img_default.pgm

clean:
	rm -rf bbrot *.o *~

.PHONY: all test clean
