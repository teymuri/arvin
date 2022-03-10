CC = gcc
CFLAGS = -O0 `pkg-config --cflags --libs glib-2.0` -g -lm -Wall -Wextra -std=c11 -pedantic -I. #-Werror
# DEPS = read.h unit_block.h let.h
OBJS =  unit.o ast.o print.o read.o type.o eval.o main.o


arvin: $(OBJS)
	$(CC) $(CFLAGS) -o arvin $(OBJS)

clean:
	rm -rf *.o *~ && clear
