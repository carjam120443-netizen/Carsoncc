CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -I src

SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:.c=.o)
TARGET := carsoncc

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET) examples/hello.car

clean:
	rm -f $(OBJECTS) $(TARGET)
