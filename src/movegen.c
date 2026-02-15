#include "bitboard.h"
#include "board.h"
#include "movegen.h"
#include "moves.h"
#include "search.h"
#include <stddef.h>
#include <string.h>


// Function pointer array for piece-specific move generation
MoveGenFunc piece_movegen[6] = {
    generate_pawn_moves,
    generate_knight_moves,
    generate_bishop_moves,
    generate_rook_moves,
    generate_queen_moves,
    generate_king_moves
};


// Helper Functions

static void add_move_if_legal(const Board* board, Square from, Square to, MoveFlags flags, MoveList* list) {
    Move m = encode_move(from, to, flags);
    if (is_legal(board, m)) {
        add_move(list, m);
    }
}


// Pawn Move Generation

void add_pawn_moves(const Board* board, Square from, Square to, MoveList* list) {
    int to_rank = square_rank(to);
    Color us = board->side_to_move;
    
    // Check if this is a promotion
    if ((us == WHITE && to_rank == 7) || (us == BLACK && to_rank == 0)) {
        add_move_if_legal(board, from, to, PROMOTION_QUEEN, list);
        add_move_if_legal(board, from, to, PROMOTION_ROOK, list);
        add_move_if_legal(board, from, to, PROMOTION_BISHOP, list);
        add_move_if_legal(board, from, to, PROMOTION_KNIGHT, list);
    } else {
        // Check if this is a capture
        Color to_color = color_on(board, to);
        if (to_color != NO_COLOR && to_color != us) {
            add_move_if_legal(board, from, to, CAPTURE, list);
        } else {
            add_move_if_legal(board, from, to, NORMAL, list);
        }
    }
}

void generate_pawn_moves(const Board* board, Square sq, MoveList* list) {
    Color us = board->side_to_move;
    Color them = (us == WHITE) ? BLACK : WHITE;
    int rank = square_rank(sq);
    int file = square_file(sq);
    
    int forward_dir = (us == WHITE) ? 8 : -8;
    int start_rank = (us == WHITE) ? 1 : 6;
    int promo_rank = (us == WHITE) ? 6 : 1;
    
    // Single push
    Square to = sq + forward_dir;
    if (to >= A1 && to <= H8 && piece_on(board, to) == NO_PIECE_TYPE) {
        add_pawn_moves(board, sq, to, list);
        
        // Double push from starting position
        if (rank == start_rank) {
            Square to2 = sq + (forward_dir * 2);
            if (piece_on(board, to2) == NO_PIECE_TYPE) {
                add_move_if_legal(board, sq, to2, NORMAL, list);
            }
        }
    }
    
    // Captures
    Bitboard attacks = pawn_attacks(sq, us);
    Bitboard enemy = board->occupied[them];
    Bitboard capture_targets = attacks & enemy;
    
    while (capture_targets) {
        Square capture_sq = pop_lsb(&capture_targets);
        add_pawn_moves(board, sq, capture_sq, list);
    }
    
    // En passant
    if (board->en_passant_square != NO_SQUARE) {
        if (attacks & square_bb(board->en_passant_square)) {
            add_move_if_legal(board, sq, board->en_passant_square, EN_PASSANT, list);
        }
    }
}


// Piece Move Generation (Non-Pawn)

void generate_knight_moves(const Board* board, Square sq, MoveList* list) {
    Color us = board->side_to_move;
    Bitboard attacks = knight_attacks(sq);
    Bitboard targets = attacks & ~board->occupied[us];
    
    while (targets) {
        Square to = pop_lsb(&targets);
        Color to_color = color_on(board, to);
        MoveFlags flags = (to_color != NO_COLOR) ? CAPTURE : NORMAL;
        add_move_if_legal(board, sq, to, flags, list);
    }
}

void generate_bishop_moves(const Board* board, Square sq, MoveList* list) {
    Color us = board->side_to_move;
    Bitboard attacks = bishop_attacks(sq, board->all_occupied);
    Bitboard targets = attacks & ~board->occupied[us];
    
    while (targets) {
        Square to = pop_lsb(&targets);
        Color to_color = color_on(board, to);
        MoveFlags flags = (to_color != NO_COLOR) ? CAPTURE : NORMAL;
        add_move_if_legal(board, sq, to, flags, list);
    }
}

void generate_rook_moves(const Board* board, Square sq, MoveList* list) {
    Color us = board->side_to_move;
    Bitboard attacks = rook_attacks(sq, board->all_occupied);
    Bitboard targets = attacks & ~board->occupied[us];
    
    while (targets) {
        Square to = pop_lsb(&targets);
        Color to_color = color_on(board, to);
        MoveFlags flags = (to_color != NO_COLOR) ? CAPTURE : NORMAL;
        add_move_if_legal(board, sq, to, flags, list);
    }
}

void generate_queen_moves(const Board* board, Square sq, MoveList* list) {
    Color us = board->side_to_move;
    Bitboard attacks = queen_attacks(sq, board->all_occupied);
    Bitboard targets = attacks & ~board->occupied[us];
    
    while (targets) {
        Square to = pop_lsb(&targets);
        Color to_color = color_on(board, to);
        MoveFlags flags = (to_color != NO_COLOR) ? CAPTURE : NORMAL;
        add_move_if_legal(board, sq, to, flags, list);
    }
}

void generate_king_moves(const Board* board, Square sq, MoveList* list) {
    Color us = board->side_to_move;
    Bitboard attacks = king_attacks(sq);
    Bitboard targets = attacks & ~board->occupied[us];
    
    while (targets) {
        Square to = pop_lsb(&targets);
        Color to_color = color_on(board, to);
        MoveFlags flags = (to_color != NO_COLOR) ? CAPTURE : NORMAL;
        add_move_if_legal(board, sq, to, flags, list);
    }
}


// Castling

void add_castling_moves(const Board* board, MoveList* list) {
    Color us = board->side_to_move;
    Color them = (us == WHITE) ? BLACK : WHITE;
    
    if (is_in_check(board, us)) {
        return;  // Cannot castle out of check
    }
    
    if (us == WHITE) {
        // White kingside
        if ((board->castling_rights & WHITE_KINGSIDE) &&
            piece_on(board, F1) == NO_PIECE_TYPE &&
            piece_on(board, G1) == NO_PIECE_TYPE &&
            !is_square_attacked(board, F1, them) &&
            !is_square_attacked(board, G1, them)) {
            add_move_if_legal(board, E1, G1, CASTLE_KINGSIDE, list);
        }
        
        // White queenside
        if ((board->castling_rights & WHITE_QUEENSIDE) &&
            piece_on(board, D1) == NO_PIECE_TYPE &&
            piece_on(board, C1) == NO_PIECE_TYPE &&
            piece_on(board, B1) == NO_PIECE_TYPE &&
            !is_square_attacked(board, D1, them) &&
            !is_square_attacked(board, C1, them)) {
            add_move_if_legal(board, E1, C1, CASTLE_QUEENSIDE, list);
        }
    } else {
        // Black kingside
        if ((board->castling_rights & BLACK_KINGSIDE) &&
            piece_on(board, F8) == NO_PIECE_TYPE &&
            piece_on(board, G8) == NO_PIECE_TYPE &&
            !is_square_attacked(board, F8, them) &&
            !is_square_attacked(board, G8, them)) {
            add_move_if_legal(board, E8, G8, CASTLE_KINGSIDE, list);
        }
        
        // Black queenside
        if ((board->castling_rights & BLACK_QUEENSIDE) &&
            piece_on(board, D8) == NO_PIECE_TYPE &&
            piece_on(board, C8) == NO_PIECE_TYPE &&
            piece_on(board, B8) == NO_PIECE_TYPE &&
            !is_square_attacked(board, D8, them) &&
            !is_square_attacked(board, C8, them)) {
            add_move_if_legal(board, E8, C8, CASTLE_QUEENSIDE, list);
        }
    }
}


// Move Generation

void generate_moves(const Board* board, MoveList* list) {
    init_move_list(list);
    Color us = board->side_to_move;
    
    // Generate moves for each piece type
    for (int piece_type = PAWN; piece_type <= KING; piece_type++) {
        Bitboard pieces = board->pieces[us][piece_type];
        
        while (pieces) {
            Square sq = pop_lsb(&pieces);
            piece_movegen[piece_type](board, sq, list);
        }
    }
    
    // Add castling moves
    add_castling_moves(board, list);
}

void generate_captures(const Board* board, MoveList* list) {
    init_move_list(list);
    Color us = board->side_to_move;
    Color them = (us == WHITE) ? BLACK : WHITE;
    
    // Generate only capture moves
    for (int piece_type = PAWN; piece_type <= KING; piece_type++) {
        Bitboard pieces = board->pieces[us][piece_type];
        
        while (pieces) {
            Square sq = pop_lsb(&pieces);
            
            if (piece_type == PAWN) {
                // Pawn captures
                Bitboard attacks = pawn_attacks(sq, us);
                Bitboard capture_targets = attacks & board->occupied[them];
                
                while (capture_targets) {
                    Square to = pop_lsb(&capture_targets);
                    add_pawn_moves(board, sq, to, list);
                }
                
                // En passant
                if (board->en_passant_square != NO_SQUARE) {
                    if (attacks & square_bb(board->en_passant_square)) {
                        add_move_if_legal(board, sq, board->en_passant_square, EN_PASSANT, list);
                    }
                }
            } else {
                // Non-pawn pieces
                Bitboard attacks;
                switch (piece_type) {
                    case KNIGHT: attacks = knight_attacks(sq); break;
                    case BISHOP: attacks = bishop_attacks(sq, board->all_occupied); break;
                    case ROOK: attacks = rook_attacks(sq, board->all_occupied); break;
                    case QUEEN: attacks = queen_attacks(sq, board->all_occupied); break;
                    case KING: attacks = king_attacks(sq); break;
                    default: attacks = 0ULL;
                }
                
                Bitboard capture_targets = attacks & board->occupied[them];
                
                while (capture_targets) {
                    Square to = pop_lsb(&capture_targets);
                    add_move_if_legal(board, sq, to, CAPTURE, list);
                }
            }
        }
    }
}

void generate_quiet_moves(const Board* board, MoveList* list) {
    init_move_list(list);
    Color us = board->side_to_move;
    
    // Generate only quiet (non-capture) moves
    for (int piece_type = PAWN; piece_type <= KING; piece_type++) {
        Bitboard pieces = board->pieces[us][piece_type];
        
        while (pieces) {
            Square sq = pop_lsb(&pieces);
            
            if (piece_type == PAWN) {
                // Pawn pushes
                int forward_dir = (us == WHITE) ? 8 : -8;
                int start_rank = (us == WHITE) ? 1 : 6;
                
                Square to = sq + forward_dir;
                if (to >= A1 && to <= H8 && piece_on(board, to) == NO_PIECE_TYPE) {
                    add_pawn_moves(board, sq, to, list);
                    
                    // Double push
                    if (square_rank(sq) == start_rank) {
                        Square to2 = sq + (forward_dir * 2);
                        if (piece_on(board, to2) == NO_PIECE_TYPE) {
                            add_move_if_legal(board, sq, to2, NORMAL, list);
                        }
                    }
                }
            } else {
                // Non-pawn pieces
                Bitboard attacks;
                switch (piece_type) {
                    case KNIGHT: attacks = knight_attacks(sq); break;
                    case BISHOP: attacks = bishop_attacks(sq, board->all_occupied); break;
                    case ROOK: attacks = rook_attacks(sq, board->all_occupied); break;
                    case QUEEN: attacks = queen_attacks(sq, board->all_occupied); break;
                    case KING: attacks = king_attacks(sq); break;
                    default: attacks = 0ULL;
                }
                
                Bitboard quiet_targets = attacks & ~board->all_occupied;
                
                while (quiet_targets) {
                    Square to = pop_lsb(&quiet_targets);
                    add_move_if_legal(board, sq, to, NORMAL, list);
                }
            }
        }
    }
    
    // Add castling moves
    add_castling_moves(board, list);
}


// Move Ordering

int mvv_lva_score(const Board* board, Move move) {
    Square to = move_to(move);
    Square from = move_from(move);
    
    PieceType victim = piece_on(board, to);
    PieceType attacker = piece_on(board, from);
    
    if (victim == NO_PIECE_TYPE) {
        return 0;
    }
    
    // MVV-LVA: (10 * victim_value - attacker_value)
    int victim_values[6] = {100, 320, 330, 500, 900, 20000};
    int attacker_values[6] = {100, 320, 330, 500, 900, 20000};
    
    return 10 * victim_values[victim] - attacker_values[attacker];
}

int score_move(const Board* board, Move move, Move hash_move, int ply) {
    // Hash move gets highest priority
    if (move == hash_move) {
        return SCORE_HASH_MOVE;
    }
    
    MoveFlags flags = move_flags(move);
    
    // Captures
    if (flags == CAPTURE || flags == EN_PASSANT) {
        return SCORE_MVV_LVA_BASE + mvv_lva_score(board, move);
    }
    
    // Promotions
    if (is_promotion(move)) {
        PieceType promo = promotion_piece(move);
        if (promo == QUEEN) return SCORE_MVV_LVA_BASE + 900;
        if (promo == ROOK) return SCORE_MVV_LVA_BASE + 500;
        if (promo == BISHOP) return SCORE_MVV_LVA_BASE + 330;
        if (promo == KNIGHT) return SCORE_MVV_LVA_BASE + 320;
    }
    
    // Killer moves
    if (move == killer_moves[ply][0]) {
        return SCORE_KILLER_1;
    }
    if (move == killer_moves[ply][1]) {
        return SCORE_KILLER_2;
    }
    
    // History heuristic
    Square from = move_from(move);
    Square to = move_to(move);
    return history_table[board->side_to_move][from][to];
}

void order_moves(const Board* board, MoveList* list, Move hash_move, int ply) {
    // Score all moves
    int scores[MAX_MOVES];
    for (int i = 0; i < list->count; i++) {
        scores[i] = score_move(board, list->moves[i], hash_move, ply);
    }
    
    // Simple selection sort (good enough for small lists)
    for (int i = 0; i < list->count - 1; i++) {
        int best_idx = i;
        int best_score = scores[i];
        
        for (int j = i + 1; j < list->count; j++) {
            if (scores[j] > best_score) {
                best_score = scores[j];
                best_idx = j;
            }
        }
        
        if (best_idx != i) {
            // Swap moves
            Move temp_move = list->moves[i];
            list->moves[i] = list->moves[best_idx];
            list->moves[best_idx] = temp_move;
            
            // Swap scores
            int temp_score = scores[i];
            scores[i] = scores[best_idx];
            scores[best_idx] = temp_score;
        }
    }
}


// Scored Move List Utilities

void init_scored_move_list(ScoredMoveList* list) {
    list->count = 0;
}

void add_scored_move(ScoredMoveList* list, Move move, int score) {
    if (list->count < MAX_MOVES) {
        list->moves[list->count].move = move;
        list->moves[list->count].score = score;
        list->count++;
    }
}

void sort_moves(ScoredMoveList* list) {
    // Selection sort
    for (int i = 0; i < list->count - 1; i++) {
        int best_idx = i;
        int best_score = list->moves[i].score;
        
        for (int j = i + 1; j < list->count; j++) {
            if (list->moves[j].score > best_score) {
                best_score = list->moves[j].score;
                best_idx = j;
            }
        }
        
        if (best_idx != i) {
            ScoredMove temp = list->moves[i];
            list->moves[i] = list->moves[best_idx];
            list->moves[best_idx] = temp;
        }
    }
}
