# Zugzwang

Chess Engine (no GUI)

## Project Overview

**Zugzwang** is a chess engine, which implements:
- **Bitboard representation** for efficient board state encoding
- **Magic bitboards** for fast sliding piece attack generation
- **Minimax algorithm with alpha-beta pruning** for optimal move selection
- **Zobrist hashing** for position transposition and repetition detection
- **Principal variation tracking** for iterative deepening
- **Piece-square tables** for fast position evaluation
- **Move ordering heuristics** (MVV-LVA, killer moves, history heuristic)

The engine outputs moves in algebraic notation using a two-column format (White on left, Black on right).

## Project Structure

```
.
├── src/                          # Source code directory
│   ├── types.h                   # Core type definitions and constants
│   ├── bitboard.h/.c             # Bitboard operations and attack generation
│   ├── board.h/.c                # Board state management with undo support
│   ├── moves.h/.c                # Move encoding and utilities
│   ├── movegen.h/.c              # Move generation with ordering
│   ├── evaluation.h/.c           # Position evaluation with tapered eval
│   ├── search.h/.c               # Negamax search with transposition table
│   ├── transposition.h/.c        # Transposition table implementation
│   ├── notation.h/.c             # Algebraic notation parsing and printing
│   ├── zobrist.h/.c              # Zobrist hashing implementation
│   └── main.c                    # Entry point and game loop
│
├── Makefile                      # Build configuration
├── README.md                     # This file
├── LICENSE                       # Project license
├── .gitattributes                # This is not a TeX repository
├── .gitignore                    # Git ignore patterns
├── Chess_Engine_Tutorial.tex     # LaTeX tutorial document (theory)
└── Chess_Engine_Tutorial.pdf     # Compiled PDF tutorial
```

## Documentation

The project includes a comprehensive tutorial document that explains all algorithmic techniques used in the engine:

- **Chess_Engine_Tutorial.tex**: LaTeX source file with formal mathematical descriptions of all algorithms
- **Chess_Engine_Tutorial.pdf**: Compiled PDF for easy reading

Topics covered include:
1. Chess algebraic notation
2. Bitboards and magic bitboards
3. Minimax with alpha-beta pruning
4. Move generation (pseudo-legal with legal filtering)
5. Zobrist hashing
6. Principal variation
7. Piece-square tables
8. Move ordering optimizations
9. The optimized search framework (negamax with transposition tables)

## Building the Engine

### Prerequisites

- GCC compiler (C11 or later)
- Make

### Compilation

```bash
make
```

This produces the `zugzwang` binary.

### Clean Build

```bash
make clean      # Remove object files and binary
make rebuild    # Clean and rebuild
```

## Usage

```bash
./zugzwang
```

The engine will:
1. Initialize the board to the starting position
2. Perform iterative deepening search
3. Output moves in two-column algebraic notation

### Move Format

Moves are displayed in standard algebraic notation:
- Simple moves: `e4`, `Nf3`
- Captures: `Bxc4`, `exd5`
- Castling: `O-O` (kingside), `O-O-O` (queenside)
- Promotion: `e8=Q`
- Check: `Nf7+`
- Checkmate: `Qh5#`

## Testing

The project includes a test suite for each source file in the `tests/` directory.

### Running Tests

To test a specific module:

```bash
make test TESTFILE=<filename>
```

### Examples

```bash
# Test bitboard module
make test TESTFILE=bitboard

# Test board module
make test TESTFILE=board
```

### Test File Naming Convention

Test files follow the naming pattern `test_<module>.c` and are located in the `tests/` directory:

- `tests/test_bitboard.c` - Tests for `src/bitboard.c`
- `tests/test_board.c` - Tests for `src/board.c`
- etc.

## Acknowledgments

Special thanks to [the Chess Programming Wiki](https://www.chessprogramming.org/Main_Page) for extensive documentation and resources that made this implementation possible.
