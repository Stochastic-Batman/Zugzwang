#include "evaluation.h"
#include "bitboard.h"
#include "board.h"
#include "movegen.h"
#include <stdlib.h>

// Piece-Square Tables (from White's perspective)

int pawn_pst[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     50,  50,  50,  50,  50,  50,  50,  50,
     10,  10,  20,  30,  30,  20,  10,  10,
      5,   5,  10,  25,  25,  10,   5,   5,
      0,   0,   0,  20,  20,   0,   0,   0,
      5,  -5, -10,   0,   0, -10,  -5,   5,
      5,  10,  10, -20, -20,  10,  10,   5,
      0,   0,   0,   0,   0,   0,   0,   0
};

int knight_pst[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

int bishop_pst[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

int rook_pst[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
      5,  10,  10,  10,  10,  10,  10,   5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
      0,   0,   0,   5,   5,   0,   0,   0
};

int queen_pst[64] = {
    -20, -10, -10,  -5,  -5, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,   5,   5,   5,   0, -10,
     -5,   0,   5,   5,   5,   5,   0,  -5,
      0,   0,   5,   5,   5,   5,   0,  -5,
    -10,   5,   5,   5,   5,   5,   0, -10,
    -10,   0,   5,   0,   0,   0,   0, -10,
    -20, -10, -10,  -5,  -5, -10, -10, -20
};

int king_pst_midgame[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
     20,  20,   0,   0,   0,   0,  20,  20,
     20,  30,  10,   0,   0,  10,  30,  20
};

int king_pst_endgame[64] = {
    -50, -40, -30, -20, -20, -30, -40, -50,
    -30, -20, -10,   0,   0, -10, -20, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -30,   0,   0,   0,   0, -30, -30,
    -50, -30, -30, -30, -30, -30, -30, -50
};


// Utility Functions

int piece_value(PieceType piece) {
    switch (piece) {
        case PAWN:   return PAWN_VALUE;
        case KNIGHT: return KNIGHT_VALUE;
        case BISHOP: return BISHOP_VALUE;
        case ROOK:   return ROOK_VALUE;
        case QUEEN:  return QUEEN_VALUE;
        case KING:   return KING_VALUE;
        default:     return 0;
    }
}

Square mirror_square(Square sq) {
    return sq ^ 56;  // XOR with 56 flips the rank
}

int get_game_phase(const Board* board) {
    // Calculate non-pawn material for both sides
    int material = 0;
    
    for (int color = WHITE; color <= BLACK; color++) {
        material += popcount(board->pieces[color][KNIGHT]) * KNIGHT_VALUE;
        material += popcount(board->pieces[color][BISHOP]) * BISHOP_VALUE;
        material += popcount(board->pieces[color][ROOK]) * ROOK_VALUE;
        material += popcount(board->pieces[color][QUEEN]) * QUEEN_VALUE;
    }
    
    // Starting material (excluding pawns): 4*(N+B) + 4*R + 2*Q = 6400
    const int max_material = 6400;
    
    // Phase goes from 256 (opening) to 0 (endgame)
    if (material >= max_material) return 256;
    return (material * 256) / max_material;
}

int tapered_eval(int mg_score, int eg_score, int phase) {
    // Interpolate between middlegame and endgame based on phase
    // phase = 256 (opening) -> return mg_score
    // phase = 0 (endgame) -> return eg_score
    return ((mg_score * phase) + (eg_score * (256 - phase))) / 256;
}

bool is_endgame(const Board* board) {
    return get_game_phase(board) < 64;  // Less than 25% material remaining
}


// Evaluation Components

int evaluate_material(const Board* board) {
    int score = 0;
    
    for (int piece_type = PAWN; piece_type <= QUEEN; piece_type++) {
        int white_count = popcount(board->pieces[WHITE][piece_type]);
        int black_count = popcount(board->pieces[BLACK][piece_type]);
        score += (white_count - black_count) * piece_value(piece_type);
    }
    
    return score;
}

int evaluate_piece_square(const Board* board) {
    int mg_score = 0;
    int eg_score = 0;
    
    int* pst_tables[6] = {pawn_pst, knight_pst, bishop_pst, rook_pst, queen_pst, king_pst_midgame};
    
    for (int piece_type = PAWN; piece_type <= KING; piece_type++) {
        Bitboard white_pieces = board->pieces[WHITE][piece_type];
        while (white_pieces) {
            Square sq = pop_lsb(&white_pieces);
            
            if (piece_type == KING) {
                mg_score += king_pst_midgame[sq];
                eg_score += king_pst_endgame[sq];
            } else {
                mg_score += pst_tables[piece_type][sq];
                eg_score += pst_tables[piece_type][sq];
            }
        }
        
        Bitboard black_pieces = board->pieces[BLACK][piece_type];
        while (black_pieces) {
            Square sq = pop_lsb(&black_pieces);
            Square mirrored = mirror_square(sq);
            
            if (piece_type == KING) {
                mg_score -= king_pst_midgame[mirrored];
                eg_score -= king_pst_endgame[mirrored];
            } else {
                mg_score -= pst_tables[piece_type][mirrored];
                eg_score -= pst_tables[piece_type][mirrored];
            }
        }
    }
    
    // Taper between midgame and endgame
    int phase = get_game_phase(board);
    return tapered_eval(mg_score, eg_score, phase);
}

int evaluate_mobility(const Board* board) {
    int score = 0;
    
    MoveList white_moves, black_moves;
    Color original_side = board->side_to_move;
    
    // Count mobility
    Board temp;
    copy_board(&temp, board);
    temp.side_to_move = WHITE;
    generate_moves(&temp, &white_moves);
    
    copy_board(&temp, board);
    temp.side_to_move = BLACK;
    generate_moves(&temp, &black_moves);
    
    // Mobility bonus (1 centipawn per move)
    return white_moves.count - black_moves.count;
}

int evaluate_pawn_structure(const Board* board) {
    int score = 0;
    
    // Doubled pawns penalty
    for (int file = 0; file < 8; file++) {
        Bitboard file_mask = file_mask(file);
        
        int white_pawns = popcount(board->pieces[WHITE][PAWN] & file_mask);
        int black_pawns = popcount(board->pieces[BLACK][PAWN] & file_mask);
        
        if (white_pawns > 1) score -= (white_pawns - 1) * 10;
        if (black_pawns > 1) score += (black_pawns - 1) * 10;
    }
    
    // Isolated pawns penalty (simplified)
    for (int file = 0; file < 8; file++) {
        Bitboard file_mask = file_mask(file);
        Bitboard adjacent_files = 0ULL;
        if (file > 0) adjacent_files |= file_mask(file - 1);
        if (file < 7) adjacent_files |= file_mask(file + 1);
        
        // White isolated pawns
        if ((board->pieces[WHITE][PAWN] & file_mask) && 
            !(board->pieces[WHITE][PAWN] & adjacent_files)) {
            score -= 15;
        }
        
        // Black isolated pawns
        if ((board->pieces[BLACK][PAWN] & file_mask) && 
            !(board->pieces[BLACK][PAWN] & adjacent_files)) {
            score += 15;
        }
    }
    
    return score;
}

int evaluate_king_safety(const Board* board) {
    int score = 0;
    
    // Simple king safety: penalize king on open files
    Square white_king = get_king_square(board, WHITE);
    Square black_king = get_king_square(board, BLACK);
    
    if (white_king != NO_SQUARE) {
        int file = square_file(white_king);
        Bitboard file_mask_w = file_mask(file);
        
        // Penalty for king on open file (no pawns)
        if (!(board->pieces[WHITE][PAWN] & file_mask_w) &&
            !(board->pieces[BLACK][PAWN] & file_mask_w)) {
            score -= 20;
        }
    }
    
    if (black_king != NO_SQUARE) {
        int file = square_file(black_king);
        Bitboard file_mask_b = file_mask(file);
        
        if (!(board->pieces[WHITE][PAWN] & file_mask_b) &&
            !(board->pieces[BLACK][PAWN] & file_mask_b)) {
            score += 20;
        }
    }
    
    return score;
}


// Main Evaluation Function

int evaluate(const Board* board) {
    int score = 0;
    score += evaluate_material(board);
    score += evaluate_piece_square(board);
    score += evaluate_mobility(board) / 2;  // Lighter, so we divide by 2
    score += evaluate_pawn_structure(board);
    score += evaluate_king_safety(board);
    return (board->side_to_move == WHITE) ? score : -score;
}


// Initialization

void init_evaluation(void) {
    // Tables are statically initialized, nothing to do
    // This function is provided for future extensions, just in case
}
