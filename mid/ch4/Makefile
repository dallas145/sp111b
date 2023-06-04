CC := gcc
CFLAGS = -std=c99 -O0 -w
TARGET = compiler

all: $(TARGET)

$(TARGET): compiler.c
				$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o *.exe compiler
