// test_notation.c
// Test suite for notation.c

#include "../src/notation.h"
#include "../src/board.h"
#include "../src/bitboard.h"
#include "../src/zobrist.h"
#include "../src/moves.h"
#include "../src/movegen.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_utility_functions() {
    assert(piece_char(PAWN) == 'P');
    assert(piece_char(KNIGHT) == 'N');
    assert(piece_char(QUEEN) == 'Q');
    
    assert(file_char(0) == 'a');
    assert(file_char(7) == 'h');
    
    assert(rank_char(0) == '1');
    assert(rank_char(7) == '8');
    
    char sq_str[3];
    square_to_str(E4, sq_str);
    assert(strcmp(sq_str, "e4") == 0);
    
    assert(str_to_square("e4") == E4);
    assert(str_to_square("a1") == A1);
    assert(str_to_square("h8") == H8);
}

void test_coordinate_notation() {
    Move move = encode_move(E2, E4, NORMAL);
    char str[16];
    move_to_coordinate(move, str);
    assert(strcmp(str, "e2e4") == 0);
    
    Move parsed = coordinate_to_move("e2e4");
    assert(move_from(parsed) == E2);
    assert(move_to(parsed) == E4);
    
    Move promo = encode_move(E7, E8, PROMOTION_QUEEN);
    move_to_coordinate(promo, str);
    assert(strcmp(str, "e7e8q") == 0);
    
    parsed = coordinate_to_move("e7e8q");
    assert(is_promotion(parsed) == true);
    assert(promotion_piece(parsed) == QUEEN);
}

void test_algebraic_simple_moves() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    Move e4 = encode_move(E2, E4, NORMAL);
    char str[16];
    move_to_algebraic(&board, e4, str);
    assert(strcmp(str, "e4") == 0 || strcmp(str, "e4+") == 0);
    
    Move nf3 = encode_move(G1, F3, NORMAL);
    move_to_algebraic(&board, nf3, str);
    assert(strcmp(str, "Nf3") == 0);
}

void test_algebraic_captures() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
    
    Move exd5 = encode_move(E4, D5, CAPTURE);
    char str[16];
    move_to_algebraic(&board, exd5, str);
    assert(strstr(str, "exd5") != NULL);
    assert(strchr(str, 'x') != NULL);
}

void test_algebraic_castling() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    
    Move wk_castle = encode_move(E1, G1, CASTLE_KINGSIDE);
    char str[16];
    move_to_algebraic(&board, wk_castle, str);
    assert(strcmp(str, "O-O") == 0 || strcmp(str, "O-O+") == 0);
    
    Move wq_castle = encode_move(E1, C1, CASTLE_QUEENSIDE);
    move_to_algebraic(&board, wq_castle, str);
    assert(strcmp(str, "O-O-O") == 0 || strcmp(str, "O-O-O+") == 0);
}

void test_algebraic_promotion() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "8/4P3/8/8/8/8/8/4K2k w - - 0 1");
    
    Move promo = encode_move(E7, E8, PROMOTION_QUEEN);
    char str[16];
    move_to_algebraic(&board, promo, str);
    assert(strstr(str, "e8=Q") != NULL);
}

void test_algebraic_disambiguation() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "8/8/8/8/8/N7/8/N3K2k w - - 0 1");
    
    Move n1c2 = encode_move(A1, C2, NORMAL);
    char str[16];
    move_to_algebraic(&board, n1c2, str);
    // Should include rank or file disambiguation
    assert(strchr(str, '1') != NULL || strchr(str, 'a') != NULL);
}

void test_algebraic_parsing() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    Move parsed = algebraic_to_move(&board, "e4");
    assert(parsed != 0);
    assert(move_to(parsed) == E4);
    
    parsed = algebraic_to_move(&board, "Nf3");
    assert(parsed != 0);
    assert(move_to(parsed) == F3);
    assert(piece_on(&board, move_from(parsed)) == KNIGHT);
}

void test_move_list_printing() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    MoveList list;
    generate_moves(&board, &list);
    
    // Just ensure it doesn't crash
    print_move_list(&board, &list);
}

int main() {
    printf("Running notation tests...\n");
    
    test_utility_functions();
    test_coordinate_notation();
    test_algebraic_simple_moves();
    test_algebraic_captures();
    test_algebraic_castling();
    test_algebraic_promotion();
    test_algebraic_disambiguation();
    test_algebraic_parsing();
    test_move_list_printing();
    
    printf("All tests passed.\n");
    return 0;
}
