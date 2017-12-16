all: t1.out

.cpp.out:
	g++ -Wall -Wextra -std=gnu++0x -g -O3 -o $@ $<
