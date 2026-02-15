#ifndef BOARD_H
#define BOARD_H

#include "types.h"

// Undo information for making/unmaking moves
typedef struct {
    uint8_t castling_rights;
    Square en_passant_square;
    int halfmove_clock;  // Number of consecutive moves without a pawn move or a capture. If it reaches 100 (50 moves for each side), the game is a draw.
    uint64_t hash;
    PieceType captured_piece;
} UndoInfo;

// Board representation
typedef struct Board {
    Bitboard pieces[2][6];  // [color][piece_type]
    Bitboard occupied[2];    // [color]
    Bitboard all_occupied;
    
    Color side_to_move;
    uint8_t castling_rights;
    Square en_passant_square;
    int halfmove_clock;
    int fullmove_number;  // total number of turns in the game.
    
    uint64_t hash;
    
    // Undo stack for move/unmake
    UndoInfo undo_stack[MAX_PLY];
    int undo_index;
    
    // Repetition detection
    uint64_t position_history[MAX_PLY];
    int history_index;
} Board;

// Board initialization
void init_board(Board* board);
void set_fen(Board* board, const char* fen);
void get_fen(const Board* board, char* fen);
void copy_board(Board* dest, const Board* src);

// Board queries
PieceType piece_on(const Board* board, Square sq);
Color color_on(const Board* board, Square sq);
bool is_square_attacked(const Board* board, Square sq, Color by_color);
bool is_in_check(const Board* board, Color color);
Square get_king_square(const Board* board, Color color);

// Move execution (with undo support)
void make_move(Board* board, Move move);
void unmake_move(Board* board, Move move);
bool make_move_if_legal(Board* board, Move move);

// Board utilities
Bitboard get_attackers(const Board* board, Square sq, Color color);
bool is_insufficient_material(const Board* board);
bool is_repetition(const Board* board);
bool is_fifty_move_draw(const Board* board);

// Game state
GameResult get_game_result(const Board* board);
bool is_terminal(const Board* board);

#endif // BOARD_H
