#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Bitboard type
typedef uint64_t Bitboard;

// Piece types
typedef enum {
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NO_PIECE_TYPE
} PieceType;

// Colors
typedef enum {
    WHITE = 0,
    BLACK = 1,
    NO_COLOR
} Color;

// Squares (A1=0, H8=63)
typedef enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQUARE
} Square;

// Castling rights (value range for bitwise operations)
typedef enum {
    WHITE_KINGSIDE = 1,
    WHITE_QUEENSIDE = 2,
    BLACK_KINGSIDE = 4,
    BLACK_QUEENSIDE = 8
} CastlingRights;

// Move encoding (16 bits: from(6) | to(6) | flags(4))
typedef uint16_t Move;

// Move flags
typedef enum {
    NORMAL = 0,
    CAPTURE = 1,
    EN_PASSANT = 2,
    CASTLE_KINGSIDE = 3,
    CASTLE_QUEENSIDE = 4,
    PROMOTION_KNIGHT = 5,
    PROMOTION_BISHOP = 6,
    PROMOTION_ROOK = 7,
    PROMOTION_QUEEN = 8
} MoveFlags;

// Transposition table entry flags
typedef enum {
    TT_EXACT = 0,   // Exact score
    TT_LOWER = 1,   // Lower bound (failed high, beta cutoff)
    TT_UPPER = 2    // Upper bound (failed low, no move improved alpha)
} TTFlag;

// Game result
typedef enum {
    ONGOING,
    WHITE_WINS,
    BLACK_WINS,
    DRAW
} GameResult;

// Constants
#define MAX_MOVES 512
#define MAX_DEPTH 64
#define MAX_PLY 128
#define INFINITE 30000
#define MATE_SCORE 29000
#define DRAW_SCORE 0

// Piece values (centipawns)
#define PAWN_VALUE 100
#define KNIGHT_VALUE 320
#define BISHOP_VALUE 330
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 20000

// Transposition table size (must be power of 2)
#define TT_SIZE_MB 64
#define TT_ENTRIES ((TT_SIZE_MB * 1024 * 1024) / sizeof(TTEntry))

// Move ordering score ranges
#define SCORE_HASH_MOVE     10000000
#define SCORE_MVV_LVA_BASE  1000000
#define SCORE_KILLER_1      100000
#define SCORE_KILLER_2      10000

#endif // TYPES_H
