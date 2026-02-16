// test_transposition.c
// Test suite for transposition.c

#include "../src/transposition.h"
#include "../src/board.h"
#include "../src/bitboard.h"
#include "../src/zobrist.h"
#include "../src/moves.h"
#include <stdio.h>
#include <assert.h>

void test_tt_initialization() {
    TranspositionTable tt;
    init_tt(&tt, 1);
    
    assert(tt.entries != NULL);
    assert(tt.size > 0);
    assert(tt.current_age == 0);
    
    free_tt(&tt);
    assert(tt.entries == NULL);
    assert(tt.size == 0);
}

void test_tt_store_probe() {
    TranspositionTable tt;
    init_tt(&tt, 1);
    
    uint64_t hash = 0x123456789ABCDEFULL;
    Move move = encode_move(E2, E4, NORMAL);
    
    store_tt(&tt, hash, 150, move, 5, TT_EXACT);
    
    TTEntry* entry = probe_tt(&tt, hash);
    assert(entry != NULL);
    assert(entry->hash == hash);
    assert(entry->score == 150);
    assert(entry->best_move == move);
    assert(entry->depth == 5);
    assert(entry->flag == TT_EXACT);
    
    free_tt(&tt);
}

void test_tt_probe_miss() {
    TranspositionTable tt;
    init_tt(&tt, 1);
    
    uint64_t hash = 0x123456789ABCDEFULL;
    TTEntry* entry = probe_tt(&tt, hash);
    
    assert(entry == NULL);
    
    free_tt(&tt);
}

void test_tt_replacement() {
    TranspositionTable tt;
    init_tt(&tt, 1);
    
    uint64_t hash = 0x123456789ABCDEFULL;
    Move move1 = encode_move(E2, E4, NORMAL);
    Move move2 = encode_move(D2, D4, NORMAL);
    
    // Store initial entry
    store_tt(&tt, hash, 100, move1, 3, TT_EXACT);
    
    // Store deeper search - should replace
    store_tt(&tt, hash, 200, move2, 5, TT_LOWER);
    
    TTEntry* entry = probe_tt(&tt, hash);
    assert(entry->score == 200);
    assert(entry->best_move == move2);
    assert(entry->depth == 5);
    
    // Store shallower search - should not replace
    store_tt(&tt, hash, 150, move1, 2, TT_UPPER);
    
    entry = probe_tt(&tt, hash);
    assert(entry->score == 200);
    assert(entry->depth == 5);
    
    free_tt(&tt);
}

void test_tt_age() {
    TranspositionTable tt;
    init_tt(&tt, 1);
    
    uint64_t hash = 0x123456789ABCDEFULL;
    Move move = encode_move(E2, E4, NORMAL);
    
    store_tt(&tt, hash, 100, move, 5, TT_EXACT);
    
    TTEntry* entry = probe_tt(&tt, hash);
    assert(entry->age == 0);
    
    age_tt(&tt);
    assert(tt.current_age == 1);
    
    // Old entry can now be replaced by shallower search
    Move move2 = encode_move(D2, D4, NORMAL);
    store_tt(&tt, hash, 50, move2, 1, TT_EXACT);
    
    entry = probe_tt(&tt, hash);
    assert(entry->score == 50);
    assert(entry->age == 1);
    
    free_tt(&tt);
}

void test_tt_clear() {
    TranspositionTable tt;
    init_tt(&tt, 1);
    
    uint64_t hash = 0x123456789ABCDEFULL;
    Move move = encode_move(E2, E4, NORMAL);
    
    store_tt(&tt, hash, 100, move, 5, TT_EXACT);
    TTEntry* entry = probe_tt(&tt, hash);
    assert(entry != NULL);
    
    clear_tt(&tt);
    assert(tt.current_age == 0);
    
    entry = probe_tt(&tt, hash);
    assert(entry == NULL);
    
    free_tt(&tt);
}

void test_tt_cutoff() {
    int score;
    TTEntry entry;
    
    // Exact score
    entry.depth = 5;
    entry.score = 150;
    entry.flag = TT_EXACT;
    assert(tt_cutoff(&entry, 4, -100, 100, &score) == true);
    assert(score == 150);
    
    // Lower bound causing beta cutoff
    entry.flag = TT_LOWER;
    entry.score = 200;
    assert(tt_cutoff(&entry, 4, -100, 100, &score) == true);
    
    // Lower bound not causing cutoff
    entry.score = 50;
    assert(tt_cutoff(&entry, 4, -100, 100, &score) == false);
    
    // Upper bound causing alpha cutoff
    entry.flag = TT_UPPER;
    entry.score = -200;
    assert(tt_cutoff(&entry, 4, -100, 100, &score) == true);
    
    // Insufficient depth
    entry.depth = 2;
    entry.flag = TT_EXACT;
    entry.score = 150;
    assert(tt_cutoff(&entry, 5, -100, 100, &score) == false);
}

void test_mate_score_adjustment() {
    // Test mate score adjustment
    int mate_in_5 = MATE_SCORE - 5;
    int adjusted = adjust_mate_score(mate_in_5, 2);
    assert(adjusted == MATE_SCORE - 3);
    
    int unadjusted = unadjust_mate_score(adjusted, 2);
    assert(unadjusted == mate_in_5);
    
    // Test negative mate score
    int mated_in_5 = -MATE_SCORE + 5;
    adjusted = adjust_mate_score(mated_in_5, 2);
    assert(adjusted == -MATE_SCORE + 3);
    
    unadjusted = unadjust_mate_score(adjusted, 2);
    assert(unadjusted == mated_in_5);
    
    // Regular scores should pass through unchanged
    int regular = 150;
    adjusted = adjust_mate_score(regular, 3);
    assert(adjusted == regular);
    
    unadjusted = unadjust_mate_score(adjusted, 3);
    assert(unadjusted == regular);
}

int main() {
    printf("Running transposition tests...\n");
    
    test_tt_initialization();
    test_tt_store_probe();
    test_tt_probe_miss();
    test_tt_replacement();
    test_tt_age();
    test_tt_clear();
    test_tt_cutoff();
    test_mate_score_adjustment();
    
    printf("All tests passed.\n");
    return 0;
}
