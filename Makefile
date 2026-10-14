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
	./hello; status=$?; rm -f hello; test "$status" -eq 50
	./$(TARGET) examples/hello.car --dump-ir > .carson-ir-test.txt
	grep -q "Carson IR v1" .carson-ir-test.txt
	grep -q "mul" .carson-ir-test.txt
	grep -q "return" .carson-ir-test.txt
	rm -f .carson-ir-test.txt
clean:
	rm -f $(OBJECTS) $(TARGET) *.carsoncc.s
