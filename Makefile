CC = gcc
CFLAGS = -O0 `pkg-config --cflags --libs glib-2.0` -g -Wall -Wextra -std=c11 -pedantic -I./src
# DEPS = read.h unit_block.h let.h
OBJS = src/ast.o src/core.o src/eval.o src/print.o src/read.o src/type.o src/main.o


let: $(OBJS)
	$(CC) $(CFLAGS) -o let $(OBJS)

clean:
	rm -rf src/*.o
