#ifndef EVALUATION_H
#define EVALUATION_H

#include "types.h"
#include "board.h"

// Main evaluation function (returns score from side-to-move perspective)
int evaluate(const Board* board);

// Evaluation components (function pointers used here)
typedef int (*EvalFunc)(const Board*);

int evaluate_material(const Board* board);
int evaluate_piece_square(const Board* board);
int evaluate_mobility(const Board* board);
int evaluate_pawn_structure(const Board* board);
int evaluate_king_safety(const Board* board);

// Piece-square tables (indexed from white's perspective)
extern int pawn_pst[64];
extern int knight_pst[64];
extern int bishop_pst[64];
extern int rook_pst[64];
extern int queen_pst[64];
extern int king_pst_midgame[64];
extern int king_pst_endgame[64];

// Evaluation utilities
bool is_endgame(const Board* board);
int piece_value(PieceType piece);
int get_game_phase(const Board* board);

// Tapered evaluation (interpolate between midgame and endgame)
int tapered_eval(int mg_score, int eg_score, int phase);

// Initialize evaluation tables
void init_evaluation(void);

// Mirror square for black pieces
Square mirror_square(Square sq);

#endif // EVALUATION_H
