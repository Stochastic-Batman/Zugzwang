#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "moves.h"
#include "board.h"

// Move list with scores for ordering
typedef struct {
    Move move;
    int score;
} ScoredMove;

typedef struct {
    ScoredMove moves[MAX_MOVES];
    int count;
} ScoredMoveList;

// Move generation (uses function pointers internally)
void generate_moves(const Board* board, MoveList* list);
void generate_captures(const Board* board, MoveList* list);
void generate_quiet_moves(const Board* board, MoveList* list);

// Piece-specific generation (function pointer targets)
typedef void (*MoveGenFunc)(const Board*, Square, MoveList*);

void generate_pawn_moves(const Board* board, Square sq, MoveList* list);
void generate_knight_moves(const Board* board, Square sq, MoveList* list);
void generate_bishop_moves(const Board* board, Square sq, MoveList* list);
void generate_rook_moves(const Board* board, Square sq, MoveList* list);
void generate_queen_moves(const Board* board, Square sq, MoveList* list);
void generate_king_moves(const Board* board, Square sq, MoveList* list);

// Function pointer array for move generation
extern MoveGenFunc piece_movegen[6];

// Special moves
void add_pawn_moves(const Board* board, Square from, Square to, MoveList* list);
void add_castling_moves(const Board* board, MoveList* list);

// Move ordering functions
void order_moves(const Board* board, MoveList* list, Move hash_move, int ply);
int score_move(const Board* board, Move move, Move hash_move, int ply);
int mvv_lva_score(const Board* board, Move move);

// Scored move list utilities
void init_scored_move_list(ScoredMoveList* list);
void add_scored_move(ScoredMoveList* list, Move move, int score);
void sort_moves(ScoredMoveList* list);

#endif // MOVEGEN_H
