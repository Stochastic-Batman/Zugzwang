#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

// Bitboard manipulation
Bitboard set_bit(Bitboard bb, Square sq);
Bitboard clear_bit(Bitboard bb, Square sq);
bool get_bit(Bitboard bb, Square sq);
int popcount(Bitboard bb);
Square lsb(Bitboard bb);
Square pop_lsb(Bitboard* bb);

// Board utilities
int square_rank(Square sq);
int square_file(Square sq);
Square make_square(int rank, int file);
Bitboard square_bb(Square sq);

// Attack generation
Bitboard pawn_attacks(Square sq, Color color);
Bitboard knight_attacks(Square sq);
Bitboard king_attacks(Square sq);
Bitboard bishop_attacks(Square sq, Bitboard occupied);
Bitboard rook_attacks(Square sq, Bitboard occupied);
Bitboard queen_attacks(Square sq, Bitboard occupied);

// Rays and masks
Bitboard rank_mask(int rank);
Bitboard file_mask(int file);
Bitboard diagonal_mask(Square sq);
Bitboard anti_diagonal_mask(Square sq);

// Initialization
void init_bitboards(void);

// Precomputed attack tables (non-sliding pieces)
extern Bitboard pawn_attack_table[2][64];
extern Bitboard knight_attack_table[64];
extern Bitboard king_attack_table[64];

// Magic bitboards for sliding pieces
extern Bitboard rook_magics[64];
extern Bitboard bishop_magics[64];
extern Bitboard rook_masks[64];
extern Bitboard bishop_masks[64];
extern int rook_shifts[64];
extern int bishop_shifts[64];

// Attack tables for magic bitboards
extern Bitboard rook_attacks_table[64][4096];
extern Bitboard bishop_attacks_table[64][512];

#endif // BITBOARD_H
