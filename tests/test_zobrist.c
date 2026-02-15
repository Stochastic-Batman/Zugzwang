// test_zobrist.c
// Test suite for zobrist.c

#include "../src/zobrist.h"
#include "../src/board.h"
#include "../src/bitboard.h"
#include "../src/moves.h"
#include <stdio.h>
#include <assert.h>

void test_zobrist_initialization() {
    init_zobrist();
    
    // All piece keys should be non-zero
    for (int color = 0; color < 2; color++) {
        for (int piece = 0; piece < 6; piece++) {
            for (int sq = 0; sq < 64; sq++) {
                assert(piece_keys[color][piece][sq] != 0);
            }
        }
    }
    
    // Castling keys should be non-zero
    for (int i = 0; i < 16; i++) {
        assert(castling_keys[i] != 0);
    }
    
    // En passant keys should be non-zero
    for (int i = 0; i < 64; i++) {
        assert(en_passant_keys[i] != 0);
    }
    
    // Side key should be non-zero
    assert(side_key != 0);
}

void test_zobrist_uniqueness() {
    init_zobrist();
    
    // Sample check: different pieces on same square should have different keys
    assert(piece_keys[WHITE][PAWN][E4] != piece_keys[WHITE][KNIGHT][E4]);
    assert(piece_keys[WHITE][PAWN][E4] != piece_keys[BLACK][PAWN][E4]);
    
    // Different squares should (likely) have different keys
    assert(piece_keys[WHITE][PAWN][E2] != piece_keys[WHITE][PAWN][E4]);
    
    // Different castling rights should have different keys
    assert(castling_keys[0] != castling_keys[1]);
    assert(castling_keys[WHITE_KINGSIDE] != castling_keys[WHITE_QUEENSIDE]);
}

void test_hash_computation() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    // Starting position should have consistent hash
    uint64_t hash1 = compute_hash(&board);
    uint64_t hash2 = compute_hash(&board);
    assert(hash1 == hash2);
    assert(hash1 != 0);
    
    // Set to same position via FEN should give same hash
    Board board2;
    set_fen(&board2, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    uint64_t hash3 = compute_hash(&board2);
    assert(hash1 == hash3);
}

void test_hash_different_positions() {
    init_bitboards();
    init_zobrist();
    
    Board board1, board2;
    
    // Different piece positions should give different hashes
    set_fen(&board1, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    set_fen(&board2, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    
    assert(board1.hash != board2.hash);
    
    // Different side to move should give different hash
    set_fen(&board1, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    set_fen(&board2, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    assert(board1.hash != board2.hash);
}

void test_hash_after_move() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    uint64_t hash_before = board.hash;
    
    // Make a move
    Move m = encode_move(E2, E4, NORMAL);
    make_move(&board, m);
    
    uint64_t hash_after_move = board.hash;
    assert(hash_before != hash_after_move);
    
    // Unmake should restore hash
    unmake_move(&board, m);
    assert(board.hash == hash_before);
}

void test_hash_consistency() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    // Make a move
    Move m = encode_move(E2, E4, NORMAL);
    make_move(&board, m);
    
    // Incremental hash should match full recomputation
    uint64_t incremental_hash = board.hash;
    uint64_t full_hash = compute_hash(&board);
    
    assert(incremental_hash == full_hash);
}

void test_hash_series_of_moves() {
    init_bitboards();
    init_zobrist();
    
    Board board;
    init_board(&board);
    
    // Make several moves
    Move moves[4] = {
        encode_move(E2, E4, NORMAL),
        encode_move(E7, E5, NORMAL),
        encode_move(G1, F3, NORMAL),
        encode_move(B8, C6, NORMAL)
    };
    
    for (int i = 0; i < 4; i++) {
        make_move(&board, moves[i]);
        
        // After each move, incremental should match full computation
        uint64_t incremental = board.hash;
        uint64_t full = compute_hash(&board);
        assert(incremental == full);
    }
    
    // Unmake all moves
    for (int i = 3; i >= 0; i--) {
        unmake_move(&board, moves[i]);
        
        uint64_t incremental = board.hash;
        uint64_t full = compute_hash(&board);
        assert(incremental == full);
    }
    
    // Should be back to starting position
    Board start;
    init_board(&start);
    assert(board.hash == start.hash);
}

int main() {
    printf("Running zobrist tests...\n");
    
    test_zobrist_initialization();
    test_zobrist_uniqueness();
    test_hash_computation();
    test_hash_different_positions();
    test_hash_after_move();
    test_hash_consistency();
    test_hash_series_of_moves();
    
    printf("All tests passed.\n");
    return 0;
}
