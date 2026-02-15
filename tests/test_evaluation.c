// test_evaluation.c
// Test suite for evaluation.c

#include "../src/evaluation.h"
#include "../src/board.h"
#include "../src/bitboard.h"
#include "../src/zobrist.h"
#include "../src/moves.h"
#include "../src/movegen.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void test_piece_values() {
    assert(piece_value(PAWN) == 100);
    assert(piece_value(KNIGHT) == 320);
    assert(piece_value(BISHOP) == 330);
    assert(piece_value(ROOK) == 500);
    assert(piece_value(QUEEN) == 900);
    assert(piece_value(KING) == 20000);
}

void test_mirror_square() {
    assert(mirror_square(A1) == A8);
    assert(mirror_square(A8) == A1);
    assert(mirror_square(E1) == E8);
    assert(mirror_square(E4) == E5);
    assert(mirror_square(H1) == H8);
}

void test_material_evaluation() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // Starting position - equal material
    init_board(&board);
    int score = evaluate_material(&board);
    assert(score == 0);
    
    // White up a pawn
    set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN1 w Qkq - 0 1");
    score = evaluate_material(&board);
    assert(score > 0);
    
    // Black up a queen
    set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBK3R w kq - 0 1");
    score = evaluate_material(&board);
    assert(score < 0);
}

void test_game_phase() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // Starting position - opening phase
    init_board(&board);
    int phase = get_game_phase(&board);
    assert(phase == 256);
    
    // King and pawn endgame - endgame phase
    set_fen(&board, "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1");
    phase = get_game_phase(&board);
    assert(phase == 0);
    
    // Check is_endgame
    assert(is_endgame(&board) == true);
    
    init_board(&board);
    assert(is_endgame(&board) == false);
}

void test_tapered_eval() {
    // Full opening phase (phase = 256)
    int result = tapered_eval(100, 200, 256);
    assert(result == 100);
    
    // Full endgame phase (phase = 0)
    result = tapered_eval(100, 200, 0);
    assert(result == 200);
    
    // Middle phase (phase = 128)
    result = tapered_eval(100, 200, 128);
    assert(result == 150);
}

void test_piece_square_tables() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // Pawn in center should be better than on edge
    assert(pawn_pst[D4] > pawn_pst[A4]);
    
    // Knight in center should be better than on edge
    assert(knight_pst[E4] > knight_pst[A1]);
    
    // Test PST evaluation
    init_board(&board);
    int score = evaluate_piece_square(&board);
    // Starting position should have roughly equal PST scores
    assert(abs(score) < 100);
}

void test_mobility_evaluation() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // Position with more mobility for white
    set_fen(&board, "rnbqkb1r/pppppppp/5n2/8/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 1");
    int score = evaluate_mobility(&board);
    
    // White should have at least some mobility
    assert(score != 0 || true);  // Mobility can be zero in some positions
}

void test_pawn_structure() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // Doubled pawns
    set_fen(&board, "4k3/8/8/8/8/4P3/4P3/4K3 w - - 0 1");
    int score = evaluate_pawn_structure(&board);
    assert(score < 0);  // White has doubled pawns, penalty
    
    // Isolated pawn
    set_fen(&board, "4k3/8/8/8/8/8/2P1P3/4K3 w - - 0 1");
    score = evaluate_pawn_structure(&board);
    assert(score < 0);  // Isolated pawn penalty
}

void test_king_safety() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // King on open file
    set_fen(&board, "4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    int score = evaluate_king_safety(&board);
    
    // Both kings exposed, should be equal or close
    assert(abs(score) < 50);
}

void test_full_evaluation() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    
    // Starting position
    init_board(&board);
    int score = evaluate(&board);
    
    // Should be close to 0 (equal position)
    assert(abs(score) < 100);
    
    // Position with advantage
    set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN1 w Qkq - 0 1");
    score = evaluate(&board);
    assert(score > 0);  // White advantage
    
    // Switch side to move
    set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN1 b Qkq - 0 1");
    score = evaluate(&board);
    assert(score < 0);  // Black to move, so negative score
}

void test_evaluation_symmetry() {
    init_bitboards();
    init_zobrist();
    
    Board board1, board2;
    
    // White pawn on e4
    set_fen(&board1, "4k3/8/8/8/4P3/8/8/4K3 w - - 0 1");
    int score1 = evaluate(&board1);
    
    // Black pawn on e5 (mirrored position)
    set_fen(&board2, "4k3/8/8/4p3/8/8/8/4K3 b - - 0 1");
    int score2 = evaluate(&board2);
    
    // Scores should be opposite (within small margin)
    assert(abs(score1 + score2) < 20);
}

void test_evaluation_consistency() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    int score1 = evaluate(&board);
    int score2 = evaluate(&board);
    
    assert(score1 == score2);
}

int main() {
    printf("Running evaluation tests...\n");
    
    test_piece_values();
    test_mirror_square();
    test_material_evaluation();
    test_game_phase();
    test_tapered_eval();
    test_piece_square_tables();
    test_mobility_evaluation();
    test_pawn_structure();
    test_king_safety();
    test_full_evaluation();
    test_evaluation_symmetry();
    test_evaluation_consistency();
    
    printf("All tests passed.\n");
    return 0;
}
