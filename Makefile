CC = gcc
CFLAGS = -O0 `pkg-config --cflags --libs glib-2.0` -g -Wall -Wextra -std=c11 -pedantic -I.
# DEPS = read.h unit_block.h let.h
OBJS = ast.o bit.o core.o eval.o print.o read.o type.o main.o
# BART = ./build_artifacts

let: $(OBJS)
	$(CC) $(CFLAGS) -o let $(OBJS)

clean:
	rm -rf *.o *~
