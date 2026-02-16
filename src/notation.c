#include "notation.h"
#include "bitboard.h"
#include "movegen.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


// Utility Functions

char piece_char(PieceType piece) {
    switch (piece) {
        case PAWN:   return 'P';
        case KNIGHT: return 'N';
        case BISHOP: return 'B';
        case ROOK:   return 'R';
        case QUEEN:  return 'Q';
        case KING:   return 'K';
        default:     return '?';
    }
}

char file_char(int file) {
    return 'a' + file;
}

char rank_char(int rank) {
    return '1' + rank;
}

void square_to_str(Square sq, char* str) {
    str[0] = file_char(square_file(sq));
    str[1] = rank_char(square_rank(sq));
    str[2] = '\0';
}

Square str_to_square(const char* str) {
    if (strlen(str) < 2) return NO_SQUARE;
    int file = str[0] - 'a';
    int rank = str[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return NO_SQUARE;
    return make_square(rank, file);
}


// Coordinate Notation (e2e4)

void move_to_coordinate(Move move, char* str) {
    Square from = move_from(move);
    Square to = move_to(move);
    
    str[0] = file_char(square_file(from));
    str[1] = rank_char(square_rank(from));
    str[2] = file_char(square_file(to));
    str[3] = rank_char(square_rank(to));
    str[4] = '\0';
    
    if (is_promotion(move)) {
        PieceType promo = promotion_piece(move);
        str[4] = tolower(piece_char(promo));
        str[5] = '\0';
    }
}

Move coordinate_to_move(const char* str) {
    if (strlen(str) < 4) return 0;
    
    Square from = str_to_square(str);
    Square to = str_to_square(str + 2);
    
    if (from == NO_SQUARE || to == NO_SQUARE) return 0;
    
    MoveFlags flags = NORMAL;
    
    // Check for promotion
    if (strlen(str) >= 5) {
        char promo_char = tolower(str[4]);
        if (promo_char == 'q') flags = PROMOTION_QUEEN;
        else if (promo_char == 'r') flags = PROMOTION_ROOK;
        else if (promo_char == 'b') flags = PROMOTION_BISHOP;
        else if (promo_char == 'n') flags = PROMOTION_KNIGHT;
    }
    
    return encode_move(from, to, flags);
}


// Algebraic Notation

void move_to_algebraic(const Board* board, Move move, char* str) {
    Square from = move_from(move);
    Square to = move_to(move);
    MoveFlags flags = move_flags(move);
    PieceType piece = piece_on(board, from);
    
    char* p = str;
    
    // Castling
    if (flags == CASTLE_KINGSIDE) {
        strcpy(str, "O-O");
        p += 3;
    } else if (flags == CASTLE_QUEENSIDE) {
        strcpy(str, "O-O-O");
        p += 5;
    } else {
        // Piece indicator (not for pawns)
        if (piece != PAWN) {
            *p++ = piece_char(piece);
            
            // Disambiguate if needed
            MoveList list;
            generate_moves((Board*)board, &list);
            
            bool need_file = false;
            bool need_rank = false;
            
            for (int i = 0; i < list.count; i++) {
                Move other = list.moves[i];
                if (other == move) continue;
                if (move_to(other) != to) continue;
                if (piece_on(board, move_from(other)) != piece) continue;
                
                Square other_from = move_from(other);
                if (square_file(other_from) == square_file(from)) {
                    need_rank = true;
                } else {
                    need_file = true;
                }
            }
            
            if (need_file) *p++ = file_char(square_file(from));
            if (need_rank) *p++ = rank_char(square_rank(from));
        }
        
        // Capture indicator
        if (is_capture(move)) {
            if (piece == PAWN) {
                *p++ = file_char(square_file(from));
            }
            *p++ = 'x';
        }
        
        // Destination square
        *p++ = file_char(square_file(to));
        *p++ = rank_char(square_rank(to));
        
        // Promotion
        if (is_promotion(move)) {
            *p++ = '=';
            *p++ = piece_char(promotion_piece(move));
        }
    }
    
    // Check/checkmate annotation
    Board temp;
    copy_board(&temp, board);
    make_move(&temp, move);
    
    if (is_in_check(&temp, temp.side_to_move)) {
        MoveList legal_moves;
        generate_moves(&temp, &legal_moves);
        
        bool has_legal = false;
        for (int i = 0; i < legal_moves.count; i++) {
            if (is_legal(&temp, legal_moves.moves[i])) {
                has_legal = true;
                break;
            }
        }
        
        *p++ = has_legal ? '+' : '#';
    }
    
    *p = '\0';
}

Move algebraic_to_move(const Board* board, const char* str) {
    // Generate all legal moves
    MoveList list;
    generate_moves((Board*)board, &list);
    
    // Try each move and see if it matches
    for (int i = 0; i < list.count; i++) {
        if (!is_legal(board, list.moves[i])) continue;
        
        char move_str[16];
        move_to_algebraic(board, list.moves[i], move_str);
        
        // Strip check/mate symbols for comparison
        int len = strlen(move_str);
        if (len > 0 && (move_str[len-1] == '+' || move_str[len-1] == '#')) {
            move_str[len-1] = '\0';
        }
        
        if (strcmp(str, move_str) == 0) {
            return list.moves[i];
        }
    }
    
    // Try coordinate notation as fallback
    return coordinate_to_move(str);
}


// Pretty Printing

void print_move(const Board* board, Move move, int move_number, Color color) {
    if (color == WHITE) {
        printf("%d. ", move_number);
    }
    
    char str[16];
    move_to_algebraic(board, move, str);
    printf("%s ", str);
    
    if (color == BLACK) {
        printf("\n");
    }
}

void print_move_list(const Board* board, const MoveList* list) {
    for (int i = 0; i < list->count; i++) {
        if (!is_legal(board, list->moves[i])) continue;
        
        char str[16];
        move_to_algebraic(board, list->moves[i], str);
        printf("%s ", str);
    }
    printf("\n");
}

void print_game_header(void) {
    printf("\n");
    printf("   White                Black\n");
    printf("-----------------------------------\n");
}

void print_two_column_move(const Board* board, Move white_move, Move black_move, int move_number) {
    printf("%2d. ", move_number);
    
    char white_str[16] = "";
    if (white_move != 0) {
        move_to_algebraic(board, white_move, white_str);
    }
    printf("%-18s", white_str);
    
    char black_str[16] = "";
    if (black_move != 0) {
        Board temp;
        copy_board(&temp, board);
        make_move(&temp, white_move);
        move_to_algebraic(&temp, black_move, black_str);
    }
    printf("%s\n", black_str);
}





// Interactive Play

Move read_user_move(const Board* board) {
    char input[32];
    
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 0;
    }
    
    // Remove newline
    input[strcspn(input, "\n")] = 0;
    
    // Handle special commands
    if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
        return 0;
    }
    
    if (strcmp(input, "help") == 0) {
        print_help_message();
        return read_user_move(board);
    }
    
    if (strcmp(input, "moves") == 0) {
        print_legal_moves(board);
        return read_user_move(board);
    }
    
    Move move = algebraic_to_move(board, input);
    if (move == 0 || !is_legal(board, move)) {
        printf("Illegal move. Try again: ");
        return read_user_move(board);
    }
    
    return move;
}

bool try_parse_move(const Board* board, const char* input, Move* out_move) {
    Move move = algebraic_to_move(board, input);
    if (move == 0 || !is_legal(board, move)) {
        return false;
    }
    *out_move = move;
    return true;
}

void print_engine_move(const Board* board, Move move, int move_number) {
    (void)move_number;
    char str[16];
    move_to_algebraic(board, move, str);
    printf("%s", str);
}

bool is_move_legal_input(const Board* board, const char* input) {
    Move move = algebraic_to_move(board, input);
    return move != 0 && is_legal(board, move);
}

void print_legal_moves(const Board* board) {
    MoveList list;
    generate_moves((Board*)board, &list);
    
    printf("\nLegal moves: ");
    int count = 0;
    for (int i = 0; i < list.count; i++) {
        if (!is_legal(board, list.moves[i])) continue;
        
        char str[16];
        move_to_algebraic(board, list.moves[i], str);
        printf("%s ", str);
        count++;
        
        if (count % 10 == 0) printf("\n             ");
    }
    printf("\n\n");
}

void print_help_message(void) {
    printf("\nChess Move Notation:\n");
    printf("  Pawn moves: e4, d5\n");
    printf("  Piece moves: Nf3, Bb5 (N=Knight, B=Bishop, R=Rook, Q=Queen, K=King)\n");
    printf("  Captures: Bxc6, exd5\n");
    printf("  Castling: O-O (kingside), O-O-O (queenside)\n");
    printf("  Promotion: e8=Q\n");
    printf("\nCommands:\n");
    printf("  moves - Show all legal moves\n");
    printf("  help  - Show this message\n");
    printf("  quit  - Exit the game\n\n");
}

Color choose_player_color(void) {
    printf("Choose your color (w/b): ");
    char input[8];
    
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return WHITE;
    }
    
    char c = tolower(input[0]);
    return (c == 'b') ? BLACK : WHITE;
}
