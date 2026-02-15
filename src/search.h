#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include "board.h"
#include "moves.h"
#include "transposition.h"
#include <stddef.h>

// Search information and statistics
typedef struct {
    int nodes_searched;
    int qnodes_searched;
    int tt_hits;
    int tt_cutoffs;
    Move best_move;
    Move pv[MAX_PLY];
    int pv_length;
    int selective_depth;
    bool time_up;
    uint64_t start_time;
    uint64_t time_limit_ms;
} SearchInfo;

// Search parameters
typedef struct {
    int max_depth;
    int aspiration_window;
    bool use_aspiration;
    bool use_quiescence;
    TranspositionTable* tt;
} SearchParams;

// Killer moves table: [ply][killer_index]
extern Move killer_moves[MAX_PLY][2];

// History heuristic table: [color][from][to]
extern int history_table[2][64][64];

// Main search functions
int negamax(Board* board, int depth, int alpha, int beta, int ply, SearchInfo* info, SearchParams* params);

int quiescence_search(Board* board, int alpha, int beta, int ply, SearchInfo* info, SearchParams* params);

Move find_best_move(Board* board, int depth, SearchInfo* info, SearchParams* params);

// Iterative deepening with aspiration windows
Move iterative_deepening(Board* board, int max_depth, SearchInfo* info, SearchParams* params);

// Heuristic updates
void update_killers(Move move, int ply);
void update_history(Move move, Color color, int depth);
void clear_heuristics(void);

// Search utilities
void init_search(SearchInfo* info);
bool is_draw(const Board* board);
int evaluate_terminal(const Board* board, int ply);
bool should_stop_search(const SearchInfo* info);

// Principal variation
void extract_pv(Board* board, TranspositionTable* tt, Move* pv, int* length);

#endif // SEARCH_H
