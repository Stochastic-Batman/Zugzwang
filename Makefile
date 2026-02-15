CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c11 -march=native
TARGET = zugzwang

SRCDIR = src
TESTDIR = tests

SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/bitboard.c \
          $(SRCDIR)/board.c \
          $(SRCDIR)/moves.c \
          $(SRCDIR)/movegen.c \
          $(SRCDIR)/evaluation.c \
          $(SRCDIR)/search.c \
          $(SRCDIR)/transposition.c \
          $(SRCDIR)/notation.c \
          $(SRCDIR)/zobrist.c

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Test target: make test TESTFILE=bitboard
# This compiles and runs tests/test_<TESTFILE>.c against src/<TESTFILE>.c
test:
ifndef TESTFILE
	@echo "Usage: make test TESTFILE=<filename>"
	@echo "Example: make test TESTFILE=bitboard"
	@exit 1
endif
	@echo "Testing $(TESTFILE)..."
	@$(CC) $(CFLAGS) -o $(TESTDIR)/test_$(TESTFILE) $(TESTDIR)/test_$(TESTFILE).c $(SRCDIR)/$(TESTFILE).c
	@$(TESTDIR)/test_$(TESTFILE)
	@rm -f $(TESTDIR)/test_$(TESTFILE)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TESTDIR)/test_*

rebuild: clean all

# Run with specific depth
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS = -Wall -Wextra -g -std=c11 -DDEBUG
debug: clean $(TARGET)

.PHONY: all clean rebuild run debug test
