#ifndef MOVES_H
#define MOVES_H

#include "types.h"

// Move encoding/decoding
Move make_move(Square from, Square to, MoveFlags flags);
Square move_from(Move move);
Square move_to(Move move);
MoveFlags move_flags(Move move);
bool is_capture(Move move);
bool is_promotion(Move move);
PieceType promotion_piece(Move move);

// Move list
typedef struct {
    Move moves[MAX_MOVES];
    int count;
} MoveList;

void init_move_list(MoveList* list);
void add_move(MoveList* list, Move move);

// Move validation
bool is_legal(const Board* board, Move move);
bool is_pseudo_legal(const Board* board, Move move);

#endif // MOVES_H
