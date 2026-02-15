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

# Test target with dependency resolution
# Usage: make test TESTFILE=<filename>
# This compiles and runs tests/test_<TESTFILE>.c with appropriate dependencies

# Define dependencies for each module
BITBOARD_DEPS = $(SRCDIR)/bitboard.c
ZOBRIST_DEPS = $(SRCDIR)/zobrist.c $(SRCDIR)/bitboard.c $(SRCDIR)/moves.c $(SRCDIR)/board.c
MOVES_DEPS = $(SRCDIR)/moves.c $(SRCDIR)/board.c $(SRCDIR)/bitboard.c $(SRCDIR)/zobrist.c
BOARD_DEPS = $(SRCDIR)/board.c $(SRCDIR)/bitboard.c $(SRCDIR)/zobrist.c $(SRCDIR)/moves.c

test:
ifndef TESTFILE
	@echo "Usage: make test TESTFILE=<filename>"
	@echo "Example: make test TESTFILE=bitboard"
	@exit 1
endif
	@echo "Testing $(TESTFILE)..."
ifeq ($(TESTFILE),bitboard)
	@$(CC) $(CFLAGS) -o $(TESTDIR)/test_$(TESTFILE) $(TESTDIR)/test_$(TESTFILE).c $(BITBOARD_DEPS)
else ifeq ($(TESTFILE),zobrist)
	@$(CC) $(CFLAGS) -o $(TESTDIR)/test_$(TESTFILE) $(TESTDIR)/test_$(TESTFILE).c $(ZOBRIST_DEPS)
else ifeq ($(TESTFILE),moves)
	@$(CC) $(CFLAGS) -o $(TESTDIR)/test_$(TESTFILE) $(TESTDIR)/test_$(TESTFILE).c $(MOVES_DEPS)
else ifeq ($(TESTFILE),board)
	@$(CC) $(CFLAGS) -o $(TESTDIR)/test_$(TESTFILE) $(TESTDIR)/test_$(TESTFILE).c $(BOARD_DEPS)
else
	@echo "Unknown test file: $(TESTFILE)"
	@echo "Trying with just $(TESTFILE).c as dependency..."
	@$(CC) $(CFLAGS) -o $(TESTDIR)/test_$(TESTFILE) $(TESTDIR)/test_$(TESTFILE).c $(SRCDIR)/$(TESTFILE).c
endif
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
