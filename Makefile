CXXFLAGS=-std=c++11 -O3

all: advent
.PHONY: all

fmt:
	pretty-mk Makefile
	clang-format -i --style=google advent.cc
	diff -u /dev/null advent.cc | fill-changed-comments
	diff -u /dev/null advent.cc | squash-strings
	clang-format -i --style=google advent.cc
.PHONY: fmt

spell:
	diff -u /dev/null advent.cc | spell
