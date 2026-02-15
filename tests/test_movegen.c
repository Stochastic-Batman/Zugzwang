// test_movegen.c
// Test suite for movegen.c

#include "../src/movegen.h"
#include "../src/board.h"
#include "../src/bitboard.h"
#include "../src/zobrist.h"
#include "../src/moves.h"
#include <stdio.h>
#include <assert.h>

void test_pawn_moves_starting_position() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    MoveList list;
    generate_pawn_moves(&board, E2, &list);
    
    assert(list.count == 2);  // e4 and e3
}

void test_knight_moves() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "8/8/8/8/3N4/8/8/8 w - - 0 1");
    
    MoveList list;
    init_move_list(&list);
    generate_knight_moves(&board, D4, &list);
    
    assert(list.count == 8);
}

void test_bishop_moves() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "8/8/8/8/3B4/8/8/8 w - - 0 1");
    
    MoveList list;
    init_move_list(&list);
    generate_bishop_moves(&board, D4, &list);
    
    assert(list.count == 13);
}

void test_rook_moves() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "8/8/8/8/3R4/8/8/8 w - - 0 1");
    
    MoveList list;
    init_move_list(&list);
    generate_rook_moves(&board, D4, &list);
    
    assert(list.count == 14);
}

void test_queen_moves() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "8/8/8/8/3Q4/8/8/8 w - - 0 1");
    
    MoveList list;
    init_move_list(&list);
    generate_queen_moves(&board, D4, &list);
    
    assert(list.count == 27);
}

void test_king_moves() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "8/8/8/8/3K4/8/8/8 w - - 0 1");
    
    MoveList list;
    init_move_list(&list);
    generate_king_moves(&board, D4, &list);
    
    assert(list.count == 8);
}

void test_generate_all_moves_starting() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    MoveList list;
    generate_moves(&board, &list);
    
    assert(list.count == 20);  // 16 pawn moves + 4 knight moves
}

void test_castling_moves() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    
    MoveList list;
    init_move_list(&list);
    add_castling_moves(&board, &list);
    
    assert(list.count == 2);  // Both white castling moves
}

void test_en_passant_generation() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
    
    MoveList list;
    init_move_list(&list);
    generate_pawn_moves(&board, E5, &list);
    
    bool found_ep = false;
    for (int i = 0; i < list.count; i++) {
        if (move_flags(list.moves[i]) == EN_PASSANT) {
            found_ep = true;
            break;
        }
    }
    
    assert(found_ep);
}

void test_pawn_promotion() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "8/4P3/8/8/8/8/8/4K2k w - - 0 1");
    
    MoveList list;
    init_move_list(&list);
    generate_pawn_moves(&board, E7, &list);
    
    assert(list.count == 4);  // Q, R, B, N promotions
}

void test_generate_captures_only() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
    
    MoveList list;
    generate_captures(&board, &list);
    
    assert(list.count > 0);
    
    // All moves should be captures
    for (int i = 0; i < list.count; i++) {
        assert(is_capture(list.moves[i]) || is_promotion(list.moves[i]));
    }
}

void test_mvv_lva_scoring() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    set_fen(&board, "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    
    // Pawn takes pawn
    Move pxe5 = encode_move(E4, E5, CAPTURE);
    int score = mvv_lva_score(&board, pxe5);
    
    assert(score > 0);
}

void test_move_ordering() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    MoveList list;
    generate_moves(&board, &list);
    
    Move hash_move = encode_move(E2, E4, NORMAL);
    order_moves(&board, &list, hash_move, 0);
    
    // Hash move should be first
    assert(list.moves[0] == hash_move);
}

void test_scored_move_list() {
    ScoredMoveList list;
    init_scored_move_list(&list);
    
    assert(list.count == 0);
    
    add_scored_move(&list, encode_move(E2, E4, NORMAL), 100);
    add_scored_move(&list, encode_move(D2, D4, NORMAL), 50);
    add_scored_move(&list, encode_move(G1, F3, NORMAL), 150);
    
    assert(list.count == 3);
    
    sort_moves(&list);
    
    assert(list.moves[0].score == 150);
    assert(list.moves[1].score == 100);
    assert(list.moves[2].score == 50);
}

int main() {
    printf("Running movegen tests...\n");
    
    test_pawn_moves_starting_position();
    test_knight_moves();
    test_bishop_moves();
    test_rook_moves();
    test_queen_moves();
    test_king_moves();
    test_generate_all_moves_starting();
    test_castling_moves();
    test_en_passant_generation();
    test_pawn_promotion();
    test_generate_captures_only();
    test_mvv_lva_scoring();
    test_move_ordering();
    test_scored_move_list();
    
    printf("All tests passed.\n");
    return 0;
}
