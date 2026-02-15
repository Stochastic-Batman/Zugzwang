#include "search.h"
#include "board.h"
#include "moves.h"
#include "movegen.h"
#include "evaluation.h"
#include "transposition.h"
#include <string.h>

// Global tables for move ordering heuristics
Move killer_moves[MAX_PLY][2];
int history_table[2][64][64];


// Heuristic Updates

void update_killers(Move move, int ply) {
    if (ply >= MAX_PLY) return;
    
    // Don't add if already first killer
    if (killer_moves[ply][0] == move) return;
    
    // Shift killers
    killer_moves[ply][1] = killer_moves[ply][0];
    killer_moves[ply][0] = move;
}

void update_history(Move move, Color color, int depth) {
    Square from = move_from(move);
    Square to = move_to(move);
    
    // Bonus based on depth squared
    history_table[color][from][to] += depth * depth;
    
    // Prevent overflow
    if (history_table[color][from][to] > 10000) {
        history_table[color][from][to] = 10000;
    }
}

void clear_heuristics(void) {
    memset(killer_moves, 0, sizeof(killer_moves));
    memset(history_table, 0, sizeof(history_table));
}


// Search Utilities

void init_search(SearchInfo* info) {
    info->nodes_searched = 0;
    info->qnodes_searched = 0;
    info->tt_hits = 0;
    info->tt_cutoffs = 0;
    info->best_move = 0;
    info->pv_length = 0;
    info->selective_depth = 0;
    info->time_up = false;
}

bool is_draw(const Board* board) {
    return is_fifty_move_draw(board) || is_repetition(board) || is_insufficient_material(board);
}

int evaluate_terminal(const Board* board, int ply) {
    // If in check, it's checkmate
    if (is_in_check(board, board->side_to_move)) {
        return -MATE_SCORE + ply;  // Negative for side to move
    }
    
    // Otherwise it's stalemate
    return DRAW_SCORE;
}

bool should_stop_search(const SearchInfo* info) {
    return info->time_up;
}


// Principal Variation

void extract_pv(Board* board, TranspositionTable* tt, Move* pv, int* length) {
    *length = 0;
    
    for (int i = 0; i < MAX_PLY; i++) {
        TTEntry* entry = probe_tt(tt, board->hash);
        
        if (!entry || entry->best_move == 0) {
            break;
        }
        
        Move move = entry->best_move;
        
        // Verify move is legal
        if (!is_legal(board, move)) {
            break;
        }
        
        pv[(*length)++] = move;
        make_move(board, move);
    }
    
    // Unmake all moves
    for (int i = *length - 1; i >= 0; i--) {
        unmake_move(board, pv[i]);
    }
}


// Quiescence Search

int quiescence_search(Board* board, int alpha, int beta, int ply, SearchInfo* info, SearchParams* params) {
    info->qnodes_searched++;
    
    // Check for draw
    if (is_draw(board)) {
        return DRAW_SCORE;
    }
    
    // Stand pat
    int stand_pat = evaluate(board);
    
    if (stand_pat >= beta) {
        return beta;
    }
    
    if (stand_pat > alpha) {
        alpha = stand_pat;
    }
    
    // Generate and search captures only
    MoveList list;
    generate_captures(board, &list);
    
    for (int i = 0; i < list.count; i++) {
        make_move(board, list.moves[i]);
        int score = -quiescence_search(board, -beta, -alpha, ply + 1, info, params);
        unmake_move(board, list.moves[i]);
        
        if (score >= beta) {
            return beta;
        }
        
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}


// Negamax Search

int negamax(Board* board, int depth, int alpha, int beta, int ply, SearchInfo* info, SearchParams* params) {
    info->nodes_searched++;
    
    // Update selective depth
    if (ply > info->selective_depth) {
        info->selective_depth = ply;
    }
    
    // Check for draw
    if (ply > 0 && is_draw(board)) {
        return DRAW_SCORE;
    }
    
    // Quiescence search at leaf nodes
    if (depth <= 0) {
        if (params->use_quiescence) {
            return quiescence_search(board, alpha, beta, ply, info, params);
        } else {
            return evaluate(board);
        }
    }
    
    // Transposition table lookup
    Move hash_move = 0;
    if (params->tt) {
        TTEntry* entry = probe_tt(params->tt, board->hash);
        if (entry) {
            info->tt_hits++;
            hash_move = entry->best_move;
            
            int tt_score;
            if (tt_cutoff(entry, depth, alpha, beta, &tt_score)) {
                info->tt_cutoffs++;
                return tt_score;
            }
        }
    }
    
    // Generate and order moves
    MoveList list;
    generate_moves(board, &list);
    
    // Check for terminal position
    if (list.count == 0) {
        return evaluate_terminal(board, ply);
    }
    
    order_moves(board, &list, hash_move, ply);
    
    // Search moves
    int best_score = -INFINITE;
    Move best_move = 0;
    TTFlag flag = TT_UPPER;
    
    for (int i = 0; i < list.count; i++) {
        make_move(board, list.moves[i]);
        int score = -negamax(board, depth - 1, -beta, -alpha, ply + 1, info, params);
        unmake_move(board, list.moves[i]);
        
        if (score > best_score) {
            best_score = score;
            best_move = list.moves[i];
            
            if (score > alpha) {
                alpha = score;
                flag = TT_EXACT;
                
                if (score >= beta) {
                    flag = TT_LOWER;
                    
                    // Update heuristics for quiet moves
                    if (!is_capture(list.moves[i])) {
                        update_killers(list.moves[i], ply);
                        update_history(list.moves[i], board->side_to_move, depth);
                    }
                    
                    break;  // Beta cutoff
                }
            }
        }
    }
    
    // Store in transposition table
    if (params->tt && best_move != 0) {
        store_tt(params->tt, board->hash, best_score, best_move, depth, flag);
    }
    
    return best_score;
}


// Find Best Move

Move find_best_move(Board* board, int depth, SearchInfo* info, SearchParams* params) {
    init_search(info);
    
    int score = negamax(board, depth, -INFINITE, INFINITE, 0, info, params);
    
    // Extract PV
    if (params->tt) {
        extract_pv(board, params->tt, info->pv, &info->pv_length);
        if (info->pv_length > 0) {
            info->best_move = info->pv[0];
        }
    }
    
    return info->best_move;
}


// Iterative Deepening

Move iterative_deepening(Board* board, int max_depth, SearchInfo* info, SearchParams* params) {
    init_search(info);
    clear_heuristics();
    
    Move best_move = 0;
    int prev_score = 0;
    
    for (int depth = 1; depth <= max_depth; depth++) {
        int alpha = -INFINITE;
        int beta = INFINITE;
        
        // Aspiration window (after depth 3)
        if (params->use_aspiration && depth > 3) {
            alpha = prev_score - params->aspiration_window;
            beta = prev_score + params->aspiration_window;
        }
        
        int score = negamax(board, depth, alpha, beta, 0, info, params);
        
        // Re-search with full window if we fell outside aspiration window
        if (params->use_aspiration && depth > 3 && (score <= alpha || score >= beta)) {
            score = negamax(board, depth, -INFINITE, INFINITE, 0, info, params);
        }
        
        prev_score = score;
        
        // Extract PV
        if (params->tt) {
            extract_pv(board, params->tt, info->pv, &info->pv_length);
            if (info->pv_length > 0) {
                best_move = info->pv[0];
                info->best_move = best_move;
            }
        }
        
        // Check if time is up
        if (should_stop_search(info)) {
            break;
        }
    }
    
    return best_move;
}
