#include "transposition.h"
#include <stdlib.h>
#include <string.h>


// Initialize transposition table

void init_tt(TranspositionTable* tt, size_t size_mb) {
    size_t num_entries = (size_mb * 1024 * 1024) / sizeof(TTEntry);
    tt->entries = (TTEntry*)calloc(num_entries, sizeof(TTEntry));
    tt->size = num_entries;
    tt->current_age = 0;
}

void free_tt(TranspositionTable* tt) {
    if (tt->entries) {
        free(tt->entries);
        tt->entries = NULL;
    }
    tt->size = 0;
}

void clear_tt(TranspositionTable* tt) {
    if (tt->entries) {
        memset(tt->entries, 0, tt->size * sizeof(TTEntry));
    }
    tt->current_age = 0;
}

void age_tt(TranspositionTable* tt) {
    tt->current_age++;
}


// Lookup and store

TTEntry* probe_tt(const TranspositionTable* tt, uint64_t hash) {
    if (!tt->entries || tt->size == 0) {
        return NULL;
    }
    
    size_t index = hash % tt->size;
    TTEntry* entry = &tt->entries[index];
    
    if (entry->hash == hash) {
        return entry;
    }
    
    return NULL;
}

void store_tt(TranspositionTable* tt, uint64_t hash, int score, Move best_move, int depth, TTFlag flag) {
    if (!tt->entries || tt->size == 0) {
        return;
    }
    
    size_t index = hash % tt->size;
    TTEntry* entry = &tt->entries[index];
    
    // Always replace or depth-preferred replacement
    if (entry->hash == 0 || entry->age != tt->current_age || depth >= entry->depth) {
        entry->hash = hash;
        entry->score = (int16_t)score;
        entry->best_move = best_move;
        entry->depth = (uint8_t)depth;
        entry->flag = (uint8_t)flag;
        entry->age = tt->current_age;
    }
}


// Utility functions

bool tt_cutoff(const TTEntry* entry, int depth, int alpha, int beta, int* score) {
    if (!entry || entry->depth < depth) {
        return false;
    }
    
    *score = entry->score;
    
    if (entry->flag == TT_EXACT) {
        return true;
    }
    
    if (entry->flag == TT_LOWER && *score >= beta) {
        return true;
    }
    
    if (entry->flag == TT_UPPER && *score <= alpha) {
        return true;
    }
    
    return false;
}

int adjust_mate_score(int score, int ply) {
    if (score > MATE_SCORE - MAX_PLY) {
        return score + ply;
    }
    if (score < -MATE_SCORE + MAX_PLY) {
        return score - ply;
    }
    return score;
}

int unadjust_mate_score(int score, int ply) {
    if (score > MATE_SCORE - MAX_PLY) {
        return score - ply;
    }
    if (score < -MATE_SCORE + MAX_PLY) {
        return score + ply;
    }
    return score;
}
