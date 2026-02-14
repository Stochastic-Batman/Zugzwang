CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c11 -march=native
TARGET = zugzwang

SRCDIR = src
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

clean:
	rm -f $(OBJECTS) $(TARGET)

rebuild: clean all

# Run with specific depth
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS = -Wall -Wextra -g -std=c11 -DDEBUG
debug: clean $(TARGET)

.PHONY: all clean rebuild run debug
