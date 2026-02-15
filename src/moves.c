#include "moves.h"
#include "board.h"

// Move Encoding/Decoding
// 
// Move format (16 bits):
// Bits 0-5:   from square (6 bits, 0-63)
// Bits 6-11:  to square (6 bits, 0-63)
// Bits 12-15: move flags (4 bits)

Move make_move(Square from, Square to, MoveFlags flags) {
    return (Move)((flags << 12) | (to << 6) | from);
}

Square move_from(Move move) {
    return (Square)(move & 63);
}

Square move_to(Move move) {
    return (Square)((move >> 6) & 63);
}

MoveFlags move_flags(Move move) {
    return (MoveFlags)(move >> 12);
}

bool is_capture(Move move) {
    MoveFlags flags = move_flags(move);
    return flags == CAPTURE || flags == EN_PASSANT;
}

bool is_promotion(Move move) {
    MoveFlags flags = move_flags(move);
    return flags >= PROMOTION_KNIGHT && flags <= PROMOTION_QUEEN;
}

PieceType promotion_piece(Move move) {
    if (!is_promotion(move)) {
        return NO_PIECE_TYPE;
    }
    
    MoveFlags flags = move_flags(move);
    switch (flags) {
        case PROMOTION_KNIGHT: return KNIGHT;
        case PROMOTION_BISHOP: return BISHOP;
        case PROMOTION_ROOK:   return ROOK;
        case PROMOTION_QUEEN:  return QUEEN;
        default:               return NO_PIECE_TYPE;
    }
}


// Move List Management

void init_move_list(MoveList* list) {
    list->count = 0;
}

void add_move(MoveList* list, Move move) {
    if (list->count < MAX_MOVES) {
        list->moves[list->count++] = move;
    }
}


// Move Validation

bool is_pseudo_legal(const Board* board, Move move) {
    Square from = move_from(move);
    Square to = move_to(move);
    MoveFlags flags = move_flags(move);
    
    if (from == NO_SQUARE || to == NO_SQUARE || from == to) {
        return false;
    }
    
    // Check that there's a piece of the correct color on the from square
    PieceType piece = piece_on(board, from);
    if (piece == NO_PIECE_TYPE) {
        return false;
    }
    
    Color piece_color = color_on(board, from);
    if (piece_color != board->side_to_move) {
        return false;
    }
    
    // For non-castling moves, check that we're not capturing our own piece
    if (flags != CASTLE_KINGSIDE && flags != CASTLE_QUEENSIDE) {
        Color to_color = color_on(board, to);
        if (to_color == board->side_to_move) {
            return false;
        }
    }
    
    return true;
}

bool is_legal(const Board* board, Move move) {
    if (!is_pseudo_legal(board, move))
        return false;
    
    Board temp;
    copy_board(&temp, board);
    make_move(&temp, move);
    
    return !is_in_check(&temp, board->side_to_move);
}
