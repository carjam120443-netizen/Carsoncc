CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -I src
SOURCES := $(filter-out src/semantic.c,$(wildcard src/*.c))
OBJECTS := $(SOURCES:.c=.o)
TARGET := carsoncc

.PHONY: all clean test
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
test: $(TARGET)
	./$(TARGET) examples/hello.car -o hello
	./hello; status=$$?; rm -f hello; test "$$status" -eq 50
clean:
	rm -f $(OBJECTS) $(TARGET) *.carsoncc.s
