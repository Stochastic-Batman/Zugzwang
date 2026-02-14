#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "types.h"
#include "moves.h"

// Transposition table entry
typedef struct {
    uint64_t hash;      // Zobrist hash of position
    int16_t score;      // Evaluation score
    Move best_move;     // Best move found
    uint8_t depth;      // Depth of search
    uint8_t flag;       // TTFlag: EXACT, LOWER, or UPPER
    uint8_t age;        // Search age for replacement scheme
} TTEntry;

// Transposition table
typedef struct {
    TTEntry* entries;
    size_t size;
    uint8_t current_age;
} TranspositionTable;

// Initialize and cleanup
void init_tt(TranspositionTable* tt, size_t size_mb);
void free_tt(TranspositionTable* tt);
void clear_tt(TranspositionTable* tt);
void age_tt(TranspositionTable* tt);

// Lookup and store
TTEntry* probe_tt(const TranspositionTable* tt, uint64_t hash);
void store_tt(TranspositionTable* tt, uint64_t hash, int score, 
              Move best_move, int depth, TTFlag flag);

// Utility functions
bool tt_cutoff(const TTEntry* entry, int depth, int alpha, int beta, int* score);
int adjust_mate_score(int score, int ply);
int unadjust_mate_score(int score, int ply);

#endif // TRANSPOSITION_H
