CC = gcc
CFLAGS = -O0 -g -lm `pkg-config --libs glib-2.0` -Wall -Wextra -pedantic -I. `pkg-config --cflags glib-2.0`
# -lreadline -lncurses
DEPS = unit.h ast.h print.h read.h type.h eval.h
OBJ = unit.o ast.o print.o read.o type.o eval.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

arvin: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o *~ && clear

memcheck:
	valgrind -s --leak-check=full --show-reachable=yes ./arvin $(script)
