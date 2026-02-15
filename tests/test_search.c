// test_search.c
// Test suite for search.c
// FULLY FIXED VERSION with all corrections

#include "../src/search.h"
#include "../src/board.h"
#include "../src/bitboard.h"
#include "../src/zobrist.h"
#include "../src/moves.h"
#include "../src/movegen.h"
#include "../src/evaluation.h"
#include "../src/transposition.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void test_init_search() {
    SearchInfo info;
    init_search(&info);
    
    assert(info.nodes_searched == 0);
    assert(info.qnodes_searched == 0);
    assert(info.tt_hits == 0);
    assert(info.tt_cutoffs == 0);
    assert(info.best_move == 0);
    assert(info.pv_length == 0);
    assert(info.selective_depth == 0);
    assert(info.time_up == false);
}

void test_clear_heuristics() {
    // Set some values
    killer_moves[0][0] = encode_move(E2, E4, NORMAL);
    killer_moves[0][1] = encode_move(D2, D4, NORMAL);
    history_table[WHITE][E2][E4] = 100;
    
    clear_heuristics();
    
    assert(killer_moves[0][0] == 0);
    assert(killer_moves[0][1] == 0);
    assert(history_table[WHITE][E2][E4] == 0);
}

void test_update_killers() {
    clear_heuristics();
    
    Move move1 = encode_move(E2, E4, NORMAL);
    Move move2 = encode_move(D2, D4, NORMAL);
    Move move3 = encode_move(G1, F3, NORMAL);
    
    update_killers(move1, 0);
    assert(killer_moves[0][0] == move1);
    
    update_killers(move2, 0);
    assert(killer_moves[0][0] == move2);
    assert(killer_moves[0][1] == move1);
    
    update_killers(move3, 0);
    assert(killer_moves[0][0] == move3);
    assert(killer_moves[0][1] == move2);
}

void test_update_history() {
    clear_heuristics();
    
    Move move = encode_move(E2, E4, NORMAL);
    
    update_history(move, WHITE, 5);
    assert(history_table[WHITE][E2][E4] == 25);  // depth^2 = 5^2 = 25
    
    update_history(move, WHITE, 3);
    assert(history_table[WHITE][E2][E4] == 34);  // 25 + 9 = 34
}

void test_is_draw() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    // Starting position is not a draw
    assert(!is_draw(&board));
    
    // Test with insufficient material (K vs K)
    set_fen(&board, "4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    assert(is_draw(&board));
}

void test_evaluate_terminal() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    // Checkmate position: Black is checkmated
    set_fen(&board, "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
    
    int score = evaluate_terminal(&board, 0);
    // Should return checkmate score
    assert(score < -MATE_SCORE + 100);
}

void test_should_stop_search() {
    SearchInfo info;
    init_search(&info);
    
    assert(!should_stop_search(&info));
    
    info.time_up = true;
    assert(should_stop_search(&info));
}

void test_negamax_finds_mate_in_one() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    // FIX #1: Corrected FEN - Scholar's Mate with Bishop on c4
    set_fen(&board, "rnbqkb1r/pppp1ppp/5n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 0 1");
    
    SearchInfo info;
    init_search(&info);  // FIX #2: Initialize info before use
    SearchParams params = {0};
    params.use_quiescence = false;
    params.use_aspiration = false;
    params.tt = NULL;
    
    int score = negamax(&board, 3, -INFINITE, INFINITE, 0, &info, &params);
    
    // Should find a winning score
    assert(score > 5000);
    assert(info.nodes_searched > 0);
}

void test_find_best_move_opening() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    // FIX #3: Add transposition table (required for find_best_move)
    TranspositionTable tt;
    init_tt(&tt, 1);
    
    SearchInfo info;
    SearchParams params = {0};
    params.use_quiescence = false;
    params.use_aspiration = false;
    params.tt = &tt;  // Provide TT so best move can be extracted
    
    Move best_move = find_best_move(&board, 3, &info, &params);
    
    // Should return a valid move
    assert(best_move != 0);
    assert(is_legal(&board, best_move));
    
    free_tt(&tt);
}

void test_quiescence_search_basic() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    // Position with captures available
    set_fen(&board, "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    
    SearchInfo info;
    init_search(&info);
    SearchParams params = {0};
    params.tt = NULL;
    
    int score = quiescence_search(&board, -INFINITE, INFINITE, 0, &info, &params);
    
    // Should return a reasonable score
    assert(abs(score) < 10000);
    assert(info.qnodes_searched > 0);
}

void test_iterative_deepening() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    // FIX #4: Add transposition table (required for iterative_deepening)
    TranspositionTable tt;
    init_tt(&tt, 1);
    
    SearchInfo info;
    SearchParams params = {0};
    params.use_quiescence = false;
    params.use_aspiration = false;
    params.tt = &tt;  // Provide TT so best move can be extracted
    
    Move best_move = iterative_deepening(&board, 3, &info, &params);
    
    // Should return a valid move
    assert(best_move != 0);
    assert(is_legal(&board, best_move));
    assert(info.nodes_searched > 0);
    
    free_tt(&tt);
}

void test_extract_pv_empty() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    Move pv[MAX_PLY];
    int length;
    
    TranspositionTable tt;
    init_tt(&tt, 1);
    
    extract_pv(&board, &tt, pv, &length);
    
    // No entries, should have length 0
    assert(length == 0);
    
    free_tt(&tt);
}

void test_negamax_with_transposition_table() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    TranspositionTable tt;
    init_tt(&tt, 1);
    
    SearchInfo info;
    SearchParams params = {0};
    params.use_quiescence = false;
    params.use_aspiration = false;
    params.tt = &tt;
    
    int score = negamax(&board, 3, -INFINITE, INFINITE, 0, &info, &params);
    
    // Should work with TT
    assert(abs(score) < 10000);
    assert(info.nodes_searched > 0);
    
    free_tt(&tt);
}

void test_search_consistency() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    SearchInfo info1, info2;
    SearchParams params = {0};
    params.use_quiescence = false;
    params.use_aspiration = false;
    params.tt = NULL;
    
    int score1 = negamax(&board, 3, -INFINITE, INFINITE, 0, &info1, &params);
    int score2 = negamax(&board, 3, -INFINITE, INFINITE, 0, &info2, &params);
    
    // Same position, same depth should give same score
    assert(score1 == score2);
}

int main() {
    printf("Running search tests...\n");
    
    test_init_search();
    test_clear_heuristics();
    test_update_killers();
    test_update_history();
    test_is_draw();
    test_evaluate_terminal();
    test_should_stop_search();
    test_negamax_finds_mate_in_one();
    test_find_best_move_opening();
    test_quiescence_search_basic();
    test_iterative_deepening();
    test_extract_pv_empty();
    test_negamax_with_transposition_table();
    test_search_consistency();
    
    printf("All tests passed.\n");
    return 0;
}
