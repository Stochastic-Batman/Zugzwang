// test_bitboard.c
// Test suite for bitboard.c

#include "../src/bitboard.h"
#include <stdio.h>
#include <assert.h>

void test_bit_operations() {
    Bitboard bb = 0;
    bb = set_bit(bb, E4);
    assert(get_bit(bb, E4) == true);
    assert(get_bit(bb, E5) == false);
    assert(popcount(bb) == 1);
    
    bb = set_bit(bb, D4);
    assert(popcount(bb) == 2);
    
    bb = clear_bit(bb, E4);
    assert(get_bit(bb, E4) == false);
    assert(popcount(bb) == 1);
}

void test_square_utilities() {
    assert(square_rank(A1) == 0);
    assert(square_rank(A8) == 7);
    assert(square_file(A1) == 0);
    assert(square_file(H1) == 7);
    
    Square sq = make_square(4, 4);
    assert(sq == E5);
    
    assert(square_bb(E4) == (1ULL << E4));
}

void test_lsb() {
    Bitboard bb = 0;
    bb = set_bit(bb, E4);
    bb = set_bit(bb, D4);
    bb = set_bit(bb, F4);
    
    assert(lsb(bb) == D4);
    
    Bitboard bb_copy = bb;
    Square sq1 = pop_lsb(&bb_copy);
    assert(sq1 == D4);
    assert(popcount(bb_copy) == 2);
    
    Square sq2 = pop_lsb(&bb_copy);
    assert(sq2 == E4);
    assert(popcount(bb_copy) == 1);
}

void test_attack_tables() {
    init_bitboards();
    
    Bitboard knight = knight_attacks(E4);
    assert(popcount(knight) == 8);
    
    Bitboard knight_corner = knight_attacks(A1);
    assert(popcount(knight_corner) == 2);
    
    Bitboard king = king_attacks(E4);
    assert(popcount(king) == 8);
    
    Bitboard king_corner = king_attacks(A1);
    assert(popcount(king_corner) == 3);
    
    Bitboard pawn_w = pawn_attacks(E4, WHITE);
    assert(popcount(pawn_w) == 2);
    
    Bitboard pawn_b = pawn_attacks(E5, BLACK);
    assert(popcount(pawn_b) == 2);
}

void test_sliding_pieces() {
    init_bitboards();
    
    Bitboard empty = 0;
    Bitboard bishop = bishop_attacks(D4, empty);
    assert(popcount(bishop) == 13);
    
    Bitboard occupied = set_bit(0, F6);
    Bitboard bishop_blocked = bishop_attacks(D4, occupied);
    assert(popcount(bishop_blocked) == 11);
    
    Bitboard rook = rook_attacks(D4, empty);
    assert(popcount(rook) == 14);
    
    occupied = set_bit(0, D6);
    occupied = set_bit(occupied, D2);
    Bitboard rook_blocked = rook_attacks(D4, occupied);
    assert(popcount(rook_blocked) == 11);
    
    Bitboard queen = queen_attacks(D4, empty);
    assert(popcount(queen) == 27);
}

void test_masks() {
    Bitboard rank0 = rank_mask(0);
    assert(popcount(rank0) == 8);
    
    Bitboard file0 = file_mask(0);
    assert(popcount(file0) == 8);
    
    Bitboard diag = diagonal_mask(E4);
    assert(popcount(diag) == 7);
    
    Bitboard anti_diag = anti_diagonal_mask(E4);
    assert(popcount(anti_diag) == 8);
}

int main() {
    printf("Running bitboard tests...\n");
    
    test_bit_operations();
    test_square_utilities();
    test_lsb();
    test_attack_tables();
    test_sliding_pieces();
    test_masks();
    
    printf("All tests passed.\n");
    return 0;
}
