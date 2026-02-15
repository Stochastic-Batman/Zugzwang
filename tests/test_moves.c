// test_moves.c
// Test suite for moves.c

#include "../src/moves.h"
#include "../src/board.h"
#include "../src/bitboard.h"
#include "../src/zobrist.h"
#include <stdio.h>
#include <assert.h>

void test_move_encoding() {
    Move m = encode_move(E2, E4, NORMAL);
    assert(move_from(m) == E2);
    assert(move_to(m) == E4);
    assert(move_flags(m) == NORMAL);
    
    Move capture = encode_move(E4, D5, CAPTURE);
    assert(move_from(capture) == E4);
    assert(move_to(capture) == D5);
    assert(move_flags(capture) == CAPTURE);
    assert(is_capture(capture) == true);
    
    Move promo = encode_move(E7, E8, PROMOTION_QUEEN);
    assert(is_promotion(promo) == true);
    assert(promotion_piece(promo) == QUEEN);
    
    Move ep = encode_move(E5, D6, EN_PASSANT);
    assert(is_capture(ep) == true);
    assert(move_flags(ep) == EN_PASSANT);
}

void test_promotion_pieces() {
    Move knight_promo = encode_move(A7, A8, PROMOTION_KNIGHT);
    assert(promotion_piece(knight_promo) == KNIGHT);
    
    Move bishop_promo = encode_move(A7, A8, PROMOTION_BISHOP);
    assert(promotion_piece(bishop_promo) == BISHOP);
    
    Move rook_promo = encode_move(A7, A8, PROMOTION_ROOK);
    assert(promotion_piece(rook_promo) == ROOK);
    
    Move queen_promo = encode_move(A7, A8, PROMOTION_QUEEN);
    assert(promotion_piece(queen_promo) == QUEEN);
    
    Move normal = encode_move(E2, E4, NORMAL);
    assert(promotion_piece(normal) == NO_PIECE_TYPE);
}

void test_castling_moves() {
    Move wk_castle = encode_move(E1, G1, CASTLE_KINGSIDE);
    assert(move_flags(wk_castle) == CASTLE_KINGSIDE);
    assert(is_capture(wk_castle) == false);
    
    Move wq_castle = encode_move(E1, C1, CASTLE_QUEENSIDE);
    assert(move_flags(wq_castle) == CASTLE_QUEENSIDE);
    
    Move bk_castle = encode_move(E8, G8, CASTLE_KINGSIDE);
    assert(move_from(bk_castle) == E8);
    assert(move_to(bk_castle) == G8);
}

void test_move_list() {
    MoveList list;
    init_move_list(&list);
    assert(list.count == 0);
    
    add_move(&list, encode_move(E2, E4, NORMAL));
    assert(list.count == 1);
    assert(move_from(list.moves[0]) == E2);
    
    add_move(&list, encode_move(D2, D4, NORMAL));
    assert(list.count == 2);
    
    // Test overflow protection
    for (int i = 0; i < MAX_MOVES; i++) {
        add_move(&list, encode_move(A1, A2, NORMAL));
    }
    assert(list.count == MAX_MOVES);
}

void test_pseudo_legal_basic() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    // Valid pawn move
    Move e2e4 = encode_move(E2, E4, NORMAL);
    assert(is_pseudo_legal(&board, e2e4) == true);
    
    // Invalid: no piece on square
    Move a1a2 = encode_move(A3, A4, NORMAL);
    assert(is_pseudo_legal(&board, a1a2) == false);
    
    // Invalid: same square
    Move same = encode_move(E2, E2, NORMAL);
    assert(is_pseudo_legal(&board, same) == false);
    
    // Invalid: wrong color
    board.side_to_move = WHITE;
    Move black_move = encode_move(E7, E5, NORMAL);
    assert(is_pseudo_legal(&board, black_move) == false);
}

int main() {
    printf("Running moves tests...\n");
    
    test_move_encoding();
    test_promotion_pieces();
    test_castling_moves();
    test_move_list();
    test_pseudo_legal_basic();
    
    printf("All tests passed.\n");
    return 0;
}
