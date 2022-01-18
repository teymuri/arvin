CC = gcc
CFLAGS = -O0 `pkg-config --cflags --libs glib-2.0` -g -Wall -Wextra -std=c11 -pedantic -I. #-Werror
# DEPS = read.h unit_block.h let.h
OBJS =  atom.o ast.o core.o print.o read.o type.o main.o
#  eval.o
# BART = ./build_artifacts

let: $(OBJS)
	$(CC) $(CFLAGS) -o let $(OBJS)

clean:
	rm -rf *.o *~ && clear
