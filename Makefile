CXXFLAGS=-std=c++11 -O3 -march=native

all: advent branch_and_bound
.PHONY: all

fmt:
	pretty-mk Makefile
	clang-format -i --style=google advent.cc branch_and_bound.cc
	diff -u /dev/null advent.cc | fill-changed-comments
	diff -u /dev/null branch_and_bound.cc | fill-changed-comments
	diff -u /dev/null advent.cc | squash-strings
	diff -u /dev/null branch_and_bound.cc | squash-strings
	clang-format -i --style=google advent.cc branch_and_bound.cc
	yapf -i advent.py
.PHONY: fmt

spell:
	diff -u /dev/null branch_and_bound.cc | spell
