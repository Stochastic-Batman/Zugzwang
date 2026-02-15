#include "bitboard.h"
#include "moves.h"
#include "zobrist.h"
#include <stdlib.h>
#include <time.h>

// Zobrist key tables
uint64_t piece_keys[2][6][64];     // [color][piece_type][square]
uint64_t castling_keys[16];        // [castling_rights bitmask]
uint64_t en_passant_keys[64];      // [square] (only files 0-7 used)
uint64_t side_key;                 // XOR this if black to move


// Random number generator for zobrist keys
// Use a simple xorshift64 generator for reproducible initialization
static uint64_t xorshift64(uint64_t* state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}


// Zobrist Initialization

void init_zobrist(void) {
    // Seed the random number generator
    // Using a fixed seed ensures reproducible zobrist keys across runs
    uint64_t seed = 1070372ULL;
    
    // Initialize piece keys
    for (int color = 0; color < 2; color++) {
        for (int piece = 0; piece < 6; piece++) {
            for (int sq = 0; sq < 64; sq++) {
                piece_keys[color][piece][sq] = xorshift64(&seed);
            }
        }
    }
    
    // Initialize castling keys (16 possible combinations of castling rights)
    for (int i = 0; i < 16; i++) {
        castling_keys[i] = xorshift64(&seed);
    }
    
    // Initialize en passant keys (one for each file)
    for (int sq = 0; sq < 64; sq++) {
        en_passant_keys[sq] = xorshift64(&seed);
    }
    
    // Initialize side to move key
    side_key = xorshift64(&seed);
}


// Hash Computation

uint64_t compute_hash(const Board* board) {
    uint64_t hash = 0ULL;
    
    // Hash all pieces on the board
    for (int color = 0; color < 2; color++) {
        for (int piece_type = 0; piece_type < 6; piece_type++) {
            Bitboard pieces = board->pieces[color][piece_type];
            
            // Iterate through all set bits (pieces of this type and color)
            while (pieces) {
                Square sq = pop_lsb(&pieces);
                hash ^= piece_keys[color][piece_type][sq];
            }
        }
    }
    
    // Hash castling rights
    hash ^= castling_keys[board->castling_rights];
    
    // Hash en passant square (if exists)
    if (board->en_passant_square != NO_SQUARE) {
        hash ^= en_passant_keys[board->en_passant_square];
    }
    
    // Hash side to move (if black)
    if (board->side_to_move == BLACK) {
        hash ^= side_key;
    }
    
    return hash;
}


// Incremental Hash Update During Move

void update_hash_move(Board* board, Move move) {
    Square from = move_from(move);
    Square to = move_to(move);
    MoveFlags flags = move_flags(move);
    Color us = board->side_to_move;
    Color them = (us == WHITE) ? BLACK : WHITE;
    
    PieceType piece = piece_on(board, from);
    uint64_t hash = board->hash;
    
    // Remove piece from origin square
    hash ^= piece_keys[us][piece][from];
    
    // Handle captures (remove captured piece)
    if (flags == CAPTURE) {
        PieceType captured = piece_on(board, to);
        hash ^= piece_keys[them][captured][to];
    }
    
    // Handle en passant capture (captured pawn is not on 'to' square)
    if (flags == EN_PASSANT) {
        Square captured_sq = (us == WHITE) ? (to - 8) : (to + 8);
        hash ^= piece_keys[them][PAWN][captured_sq];
    }
    
    // Handle castling (move the rook)
    if (flags == CASTLE_KINGSIDE) {
        Square rook_from = (us == WHITE) ? H1 : H8;
        Square rook_to = (us == WHITE) ? F1 : F8;
        hash ^= piece_keys[us][ROOK][rook_from];
        hash ^= piece_keys[us][ROOK][rook_to];
        hash ^= piece_keys[us][piece][to];  // Add king to destination
    } else if (flags == CASTLE_QUEENSIDE) {
        Square rook_from = (us == WHITE) ? A1 : A8;
        Square rook_to = (us == WHITE) ? D1 : D8;
        hash ^= piece_keys[us][ROOK][rook_from];
        hash ^= piece_keys[us][ROOK][rook_to];
        hash ^= piece_keys[us][piece][to];  // Add king to destination
    }
    // Handle promotion
    else if (is_promotion(move)) {
        PieceType promoted = promotion_piece(move);
        hash ^= piece_keys[us][promoted][to];  // Add promoted piece
    }
    // Normal move (add piece to destination)
    else {
        hash ^= piece_keys[us][piece][to];
    }
    
    // Update castling rights hash
    hash ^= castling_keys[board->castling_rights];  // Remove old rights
    
    // Update en passant hash
    if (board->en_passant_square != NO_SQUARE) {
        hash ^= en_passant_keys[board->en_passant_square];  // Remove old ep
    }
    
    // (New en passant square will be set in board.c and needs to be XORed there)
    
    // Toggle side to move
    hash ^= side_key;
    
    board->hash = hash;
}
