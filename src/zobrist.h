#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "types.h"
#include "board.h"

// Zobrist hash initialization
void init_zobrist(void);

// Hash computation
uint64_t compute_hash(const Board* board);
void update_hash_move(Board* board, Move move);

// Zobrist keys
extern uint64_t piece_keys[2][6][64];
extern uint64_t castling_keys[16];
extern uint64_t en_passant_keys[64];
extern uint64_t side_key;

#endif // ZOBRIST_H
