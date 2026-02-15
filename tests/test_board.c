// test_board.c
// Test suite for board.c

#include "../src/board.h"
#include "../src/bitboard.h"
#include "../src/zobrist.h"
#include "../src/moves.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_board_initialization() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    // Check starting position pieces
    assert(piece_on(&board, E1) == KING);
    assert(piece_on(&board, E8) == KING);
    assert(piece_on(&board, A1) == ROOK);
    assert(piece_on(&board, H8) == ROOK);
    assert(piece_on(&board, E2) == PAWN);
    assert(piece_on(&board, E7) == PAWN);
    
    // Check colors
    assert(color_on(&board, E1) == WHITE);
    assert(color_on(&board, E8) == BLACK);
    assert(color_on(&board, E4) == NO_COLOR);
    
    // Check game state
    assert(board.side_to_move == WHITE);
    assert(board.castling_rights == (WHITE_KINGSIDE | WHITE_QUEENSIDE | BLACK_KINGSIDE | BLACK_QUEENSIDE));
    assert(board.en_passant_square == NO_SQUARE);
    assert(board.halfmove_clock == 0);
    assert(board.fullmove_number == 1);
}

void test_fen_parsing() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // Test starting position
    set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    assert(board.side_to_move == WHITE);
    assert(board.castling_rights == 15);
    assert(piece_on(&board, E2) == PAWN);
    
    // Test after e4
    set_fen(&board, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    assert(board.side_to_move == BLACK);
    assert(piece_on(&board, E4) == PAWN);
    assert(color_on(&board, E4) == WHITE);
    assert(board.en_passant_square == E3);
    
    // Test position with limited castling
    set_fen(&board, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    assert(board.castling_rights == 15);
    assert(piece_on(&board, E1) == KING);
    assert(piece_on(&board, A1) == ROOK);
}

void test_fen_generation() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    const char* original_fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    set_fen(&board, original_fen);
    
    char generated_fen[128];
    get_fen(&board, generated_fen);
    
    // Parse generated FEN and compare
    Board board2;
    set_fen(&board2, generated_fen);
    
    assert(board.side_to_move == board2.side_to_move);
    assert(board.castling_rights == board2.castling_rights);
    assert(board.en_passant_square == board2.en_passant_square);
}

void test_piece_queries() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    // Test piece_on
    assert(piece_on(&board, E1) == KING);
    assert(piece_on(&board, D1) == QUEEN);
    assert(piece_on(&board, B1) == KNIGHT);
    assert(piece_on(&board, E4) == NO_PIECE_TYPE);
    
    // Test color_on
    assert(color_on(&board, A2) == WHITE);
    assert(color_on(&board, A7) == BLACK);
    assert(color_on(&board, A4) == NO_COLOR);
    
    // Test get_king_square
    assert(get_king_square(&board, WHITE) == E1);
    assert(get_king_square(&board, BLACK) == E8);
}

void test_attack_detection() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // Test starting position - e4 not attacked
    init_board(&board);
    assert(is_square_attacked(&board, E4, WHITE) == false);
    assert(is_square_attacked(&board, E4, BLACK) == false);
    
    // Test pawn attacks
    set_fen(&board, "8/8/8/8/3p4/8/8/8 w - - 0 1");
    assert(is_square_attacked(&board, C3, BLACK) == true);
    assert(is_square_attacked(&board, E3, BLACK) == true);
    assert(is_square_attacked(&board, D3, BLACK) == false);
    
    // Test knight attacks
    set_fen(&board, "8/8/8/8/3N4/8/8/8 w - - 0 1");
    assert(is_square_attacked(&board, E6, WHITE) == true);
    assert(is_square_attacked(&board, C6, WHITE) == true);
    assert(is_square_attacked(&board, E5, WHITE) == false);
}

void test_check_detection() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // Starting position - not in check
    init_board(&board);
    assert(is_in_check(&board, WHITE) == false);
    assert(is_in_check(&board, BLACK) == false);
    
    // White king in check from black queen
    set_fen(&board, "4k3/8/8/8/8/8/4q3/4K3 w - - 0 1");
    assert(is_in_check(&board, WHITE) == true);
    assert(is_in_check(&board, BLACK) == false);
    
    // Black king in check from white rook
    set_fen(&board, "4k3/4R3/8/8/8/8/8/4K3 b - - 0 1");
    assert(is_in_check(&board, BLACK) == true);
    assert(is_in_check(&board, WHITE) == false);
}

void test_make_unmake_simple() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    // Save original state
    Board original;
    copy_board(&original, &board);
    
    // Make a simple pawn move
    Move m = encode_move(E2, E4, NORMAL);
    make_move(&board, m);
    
    assert(piece_on(&board, E2) == NO_PIECE_TYPE);
    assert(piece_on(&board, E4) == PAWN);
    assert(color_on(&board, E4) == WHITE);
    assert(board.side_to_move == BLACK);
    assert(board.en_passant_square == E3);
    
    // Unmake the move
    unmake_move(&board, m);
    
    assert(piece_on(&board, E2) == PAWN);
    assert(piece_on(&board, E4) == NO_PIECE_TYPE);
    assert(board.side_to_move == WHITE);
    assert(board.hash == original.hash);
}

void test_make_unmake_capture() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
    
    uint64_t hash_before = board.hash;
    
    // Make capture
    Move m = encode_move(E4, E5, CAPTURE);
    make_move(&board, m);
    
    assert(piece_on(&board, E5) == PAWN);
    assert(color_on(&board, E5) == WHITE);
    assert(piece_on(&board, E4) == NO_PIECE_TYPE);
    
    // Unmake capture
    unmake_move(&board, m);
    
    assert(piece_on(&board, E4) == PAWN);
    assert(piece_on(&board, E5) == PAWN);
    assert(color_on(&board, E5) == BLACK);
    assert(board.hash == hash_before);
}

void test_castling() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // White kingside castling
    set_fen(&board, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    Move wk_castle = encode_move(E1, G1, CASTLE_KINGSIDE);
    make_move(&board, wk_castle);
    
    assert(piece_on(&board, G1) == KING);
    assert(piece_on(&board, F1) == ROOK);
    assert(piece_on(&board, E1) == NO_PIECE_TYPE);
    assert(piece_on(&board, H1) == NO_PIECE_TYPE);
    
    unmake_move(&board, wk_castle);
    assert(piece_on(&board, E1) == KING);
    assert(piece_on(&board, H1) == ROOK);
    
    // White queenside castling
    set_fen(&board, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    Move wq_castle = encode_move(E1, C1, CASTLE_QUEENSIDE);
    make_move(&board, wq_castle);
    
    assert(piece_on(&board, C1) == KING);
    assert(piece_on(&board, D1) == ROOK);
    assert(piece_on(&board, E1) == NO_PIECE_TYPE);
    assert(piece_on(&board, A1) == NO_PIECE_TYPE);
}

void test_en_passant() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // Set up en passant position
    set_fen(&board, "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
    
    assert(board.en_passant_square == F6);
    
    // Make en passant capture
    Move ep = encode_move(E5, F6, EN_PASSANT);
    make_move(&board, ep);
    
    assert(piece_on(&board, F6) == PAWN);
    assert(color_on(&board, F6) == WHITE);
    assert(piece_on(&board, F5) == NO_PIECE_TYPE);  // Captured pawn removed
    
    // Unmake en passant
    unmake_move(&board, ep);
    
    assert(piece_on(&board, E5) == PAWN);
    assert(piece_on(&board, F5) == PAWN);
    assert(piece_on(&board, F6) == NO_PIECE_TYPE);
}

void test_promotion() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "8/4P3/8/8/8/8/8/4K2k w - - 0 1");
    
    // Promote to queen
    Move promo = encode_move(E7, E8, PROMOTION_QUEEN);
    make_move(&board, promo);
    
    assert(piece_on(&board, E8) == QUEEN);
    assert(piece_on(&board, E7) == NO_PIECE_TYPE);
    
    unmake_move(&board, promo);
    
    assert(piece_on(&board, E7) == PAWN);
    assert(piece_on(&board, E8) == NO_PIECE_TYPE);
}

void test_castling_rights_update() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    
    assert(board.castling_rights == 15);
    
    // Move white king - should lose both white castling rights
    Move m = encode_move(E1, E2, NORMAL);
    make_move(&board, m);
    
    assert((board.castling_rights & WHITE_KINGSIDE) == 0);
    assert((board.castling_rights & WHITE_QUEENSIDE) == 0);
    assert((board.castling_rights & BLACK_KINGSIDE) != 0);
    assert((board.castling_rights & BLACK_QUEENSIDE) != 0);
    
    unmake_move(&board, m);
    assert(board.castling_rights == 15);
}

void test_halfmove_clock() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    assert(board.halfmove_clock == 0);
    
    // Knight move - should increment
    Move m1 = encode_move(G1, F3, NORMAL);
    make_move(&board, m1);
    assert(board.halfmove_clock == 1);
    
    // Knight move - should increment
    Move m2 = encode_move(B8, C6, NORMAL);
    make_move(&board, m2);
    assert(board.halfmove_clock == 2);
    
    // Pawn move - should reset
    Move m3 = encode_move(E2, E4, NORMAL);
    make_move(&board, m3);
    assert(board.halfmove_clock == 0);
}

void test_insufficient_material() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // K vs K
    set_fen(&board, "4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    assert(is_insufficient_material(&board) == true);
    
    // K+N vs K
    set_fen(&board, "4k3/8/8/8/8/8/8/4KN2 w - - 0 1");
    assert(is_insufficient_material(&board) == true);
    
    // K+B vs K
    set_fen(&board, "4k3/8/8/8/8/8/8/4KB2 w - - 0 1");
    assert(is_insufficient_material(&board) == true);
    
    // K+P vs K (not insufficient)
    set_fen(&board, "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1");
    assert(is_insufficient_material(&board) == false);
}

void test_fifty_move_rule() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    assert(is_fifty_move_draw(&board) == false);
    
    board.halfmove_clock = 99;
    assert(is_fifty_move_draw(&board) == false);
    
    board.halfmove_clock = 100;
    assert(is_fifty_move_draw(&board) == true);
    
    board.halfmove_clock = 150;
    assert(is_fifty_move_draw(&board) == true);
}

void test_repetition_detection() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    // Make moves that repeat position (Ng1-f3-g1-f3-g1)
    Move moves[4] = {
        encode_move(G1, F3, NORMAL),
        encode_move(B8, C6, NORMAL),
        encode_move(F3, G1, NORMAL),
        encode_move(C6, B8, NORMAL)
    };
    
    // First occurrence
    assert(is_repetition(&board) == false);
    
    // Make moves
    for (int i = 0; i < 4; i++) {
        make_move(&board, moves[i]);
    }
    
    // Second occurrence (still not repetition)
    assert(is_repetition(&board) == false);
    
    // Repeat again
    for (int i = 0; i < 4; i++) {
        make_move(&board, moves[i]);
    }
    
    // Third occurrence - threefold repetition
    assert(is_repetition(&board) == true);
}

int main() {
    printf("Running board tests...\n");
    
    test_board_initialization();
    test_fen_parsing();
    test_fen_generation();
    test_piece_queries();
    test_attack_detection();
    test_check_detection();
    test_make_unmake_simple();
    test_make_unmake_capture();
    test_castling();
    test_en_passant();
    test_promotion();
    test_castling_rights_update();
    test_halfmove_clock();
    test_insufficient_material();
    test_fifty_move_rule();
    test_repetition_detection();
    
    printf("All tests passed.\n");
    return 0;
}
