#ifndef NOTATION_H
#define NOTATION_H

#include "types.h"
#include "board.h"
#include "moves.h"

// Algebraic notation conversion
void move_to_algebraic(const Board* board, Move move, char* str);
Move algebraic_to_move(const Board* board, const char* str);

// Coordinate notation (e2e4)
void move_to_coordinate(Move move, char* str);
Move coordinate_to_move(const char* str);

// Pretty printing for move output
void print_move(const Board* board, Move move, int move_number, Color color);
void print_move_list(const Board* board, const MoveList* list);

// Game notation
void print_game_header(void);
void print_two_column_move(const Board* board, Move white_move, Move black_move, int move_number);

// Interactive play functions
Move read_user_move(const Board* board);
bool try_parse_move(const Board* board, const char* input, Move* out_move);
void print_engine_move(const Board* board, Move move, int move_number);
Color choose_player_color(void);

// Move input validation and help
bool is_move_legal_input(const Board* board, const char* input);
void print_legal_moves(const Board* board);
void print_help_message(void);

// Utilities
char piece_char(PieceType piece);
char file_char(int file);
char rank_char(int rank);
void square_to_str(Square sq, char* str);
Square str_to_square(const char* str);

#endif // NOTATION_H
