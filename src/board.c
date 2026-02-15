#include "bitboard.h"
#include "board.h"
#include "moves.h"
#include "zobrist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// Board Initialization

void init_board(Board* board) {
    // Standard starting position: https://rustic-chess.org/board_functionality/handling_fen_strings.html
    const char* start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    set_fen(board, start_fen);
}

void copy_board(Board* dest, const Board* src) {
    memcpy(dest, src, sizeof(Board));
}


// FEN Parsing and Generation

void set_fen(Board* board, const char* fen) {
    memset(board, 0, sizeof(Board));
    board->en_passant_square = NO_SQUARE;
    
    const char* p = fen;
    
    // 1. Parse piece placement (starting from rank 8 down to rank 1)
    int rank = 7, file = 0;
    while (*p && *p != ' ') {
        if (*p == '/') {
            rank--;
            file = 0;
        } else if (isdigit(*p)) {
            file += (*p - '0');  // Skip empty squares
        } else {
            // Parse piece character
            Color color = isupper(*p) ? WHITE : BLACK;
            char piece_char = tolower(*p);
            PieceType piece_type = NO_PIECE_TYPE;
            
            switch (piece_char) {
                case 'p': piece_type = PAWN; break;
                case 'n': piece_type = KNIGHT; break;
                case 'b': piece_type = BISHOP; break;
                case 'r': piece_type = ROOK; break;
                case 'q': piece_type = QUEEN; break;
                case 'k': piece_type = KING; break;
            }
            
            if (piece_type != NO_PIECE_TYPE) {
                Square sq = make_square(rank, file);
                board->pieces[color][piece_type] = set_bit(board->pieces[color][piece_type], sq);
            }
            
            file++;
        }
        p++;
    }
    
    // Update composite bitboards
    for (int color = 0; color < 2; color++) {
        board->occupied[color] = 0ULL;
        for (int piece = 0; piece < 6; piece++) {
            board->occupied[color] |= board->pieces[color][piece];
        }
    }
    board->all_occupied = board->occupied[WHITE] | board->occupied[BLACK];
    
    // 2. Parse side to move
    if (*p == ' ') p++;
    board->side_to_move = (*p == 'w') ? WHITE : BLACK;
    p++;
    
    // 3. Parse castling rights
    if (*p == ' ') p++;
    board->castling_rights = 0;
    if (*p != '-') {
        while (*p && *p != ' ') {
            if (*p == 'K') board->castling_rights |= WHITE_KINGSIDE;
            if (*p == 'Q') board->castling_rights |= WHITE_QUEENSIDE;
            if (*p == 'k') board->castling_rights |= BLACK_KINGSIDE;
            if (*p == 'q') board->castling_rights |= BLACK_QUEENSIDE;
            p++;
        }
    } else {
        p++;
    }
    
    // 4. Parse en passant square
    if (*p == ' ') p++;
    if (*p != '-') {
        int ep_file = p[0] - 'a';
        int ep_rank = p[1] - '1';
        board->en_passant_square = make_square(ep_rank, ep_file);
        p += 2;
    } else {
        board->en_passant_square = NO_SQUARE;
        p++;
    }
    
    // 5. Parse halfmove clock
    if (*p == ' ') p++;
    board->halfmove_clock = atoi(p);
    while (*p && *p != ' ') p++;
    
    // 6. Parse fullmove number
    if (*p == ' ') p++;
    board->fullmove_number = atoi(p);
    
    // Initialize undo and history indices
    board->undo_index = 0;
    board->history_index = 0;
    
    // Compute zobrist hash
    board->hash = compute_hash(board);
    board->position_history[board->history_index++] = board->hash;
}

void get_fen(const Board* board, char* fen) {
    char* p = fen;
    
    // 1. Piece placement
    for (int rank = 7; rank >= 0; rank--) {
        int empty = 0;
        for (int file = 0; file < 8; file++) {
            Square sq = make_square(rank, file);
            PieceType piece = piece_on(board, sq);
            
            if (piece == NO_PIECE_TYPE) {
                empty++;
            } else {
                if (empty > 0) {
                    *p++ = '0' + empty;
                    empty = 0;
                }
                
                Color color = color_on(board, sq);
                char piece_char;
                switch (piece) {
                    case PAWN:   piece_char = 'p'; break;
                    case KNIGHT: piece_char = 'n'; break;
                    case BISHOP: piece_char = 'b'; break;
                    case ROOK:   piece_char = 'r'; break;
                    case QUEEN:  piece_char = 'q'; break;
                    case KING:   piece_char = 'k'; break;
                    default:     piece_char = '?'; break;
                }
                
                if (color == WHITE) {
                    piece_char = toupper(piece_char);
                }
                *p++ = piece_char;
            }
        }
        
        if (empty > 0) {
            *p++ = '0' + empty;
        }
        
        if (rank > 0) {
            *p++ = '/';
        }
    }
    
    // 2. Side to move
    *p++ = ' ';
    *p++ = (board->side_to_move == WHITE) ? 'w' : 'b';
    
    // 3. Castling rights
    *p++ = ' ';
    if (board->castling_rights == 0) {
        *p++ = '-';
    } else {
        if (board->castling_rights & WHITE_KINGSIDE)  *p++ = 'K';
        if (board->castling_rights & WHITE_QUEENSIDE) *p++ = 'Q';
        if (board->castling_rights & BLACK_KINGSIDE)  *p++ = 'k';
        if (board->castling_rights & BLACK_QUEENSIDE) *p++ = 'q';
    }
    
    // 4. En passant square
    *p++ = ' ';
    if (board->en_passant_square == NO_SQUARE) {
        *p++ = '-';
    } else {
        int file = square_file(board->en_passant_square);
        int rank = square_rank(board->en_passant_square);
        *p++ = 'a' + file;
        *p++ = '1' + rank;
    }
    
    // 5. Halfmove clock
    p += sprintf(p, " %d", board->halfmove_clock);
    
    // 6. Fullmove number
    p += sprintf(p, " %d", board->fullmove_number);
    
    *p = '\0';
}


// Board Queries

PieceType piece_on(const Board* board, Square sq) {
    for (int piece = 0; piece < 6; piece++) {
        if (get_bit(board->pieces[WHITE][piece], sq)) return (PieceType)piece;
        if (get_bit(board->pieces[BLACK][piece], sq)) return (PieceType)piece;
    }
    return NO_PIECE_TYPE;
}

Color color_on(const Board* board, Square sq) {
    if (get_bit(board->occupied[WHITE], sq)) return WHITE;
    if (get_bit(board->occupied[BLACK], sq)) return BLACK;
    return NO_COLOR;
}

Square get_king_square(const Board* board, Color color) {
    Bitboard king_bb = board->pieces[color][KING];
    if (king_bb == 0) return NO_SQUARE;
    return lsb(king_bb);
}

bool is_square_attacked(const Board* board, Square sq, Color by_color) {
    Bitboard occupied = board->all_occupied;
    
    Bitboard pawn_attackers = pawn_attacks(sq, by_color == WHITE ? BLACK : WHITE);
    if (pawn_attackers & board->pieces[by_color][PAWN]) {
        return true;
    }
    
    Bitboard knight_attackers = knight_attacks(sq);
    if (knight_attackers & board->pieces[by_color][KNIGHT]) {
        return true;
    }
    
    Bitboard king_attackers = king_attacks(sq);
    if (king_attackers & board->pieces[by_color][KING]) {
        return true;
    }
    
    Bitboard bishop_attackers = bishop_attacks(sq, occupied);
    if (bishop_attackers & (board->pieces[by_color][BISHOP] | board->pieces[by_color][QUEEN])) {
        return true;
    }
    
    Bitboard rook_attackers = rook_attacks(sq, occupied);
    if (rook_attackers & (board->pieces[by_color][ROOK] | board->pieces[by_color][QUEEN])) {
        return true;
    }
    
    return false;
}

bool is_in_check(const Board* board, Color color) {
    Square king_sq = get_king_square(board, color);
    if (king_sq == NO_SQUARE) return false;
    
    Color opponent = (color == WHITE) ? BLACK : WHITE;
    return is_square_attacked(board, king_sq, opponent);
}

Bitboard get_attackers(const Board* board, Square sq, Color color) {
    Bitboard attackers = 0ULL;
    Bitboard occupied = board->all_occupied;
    
    Bitboard pawn_atk = pawn_attacks(sq, color == WHITE ? BLACK : WHITE);
    attackers |= pawn_atk & board->pieces[color][PAWN];
    
    Bitboard knight_atk = knight_attacks(sq);
    attackers |= knight_atk & board->pieces[color][KNIGHT];
    
    Bitboard king_atk = king_attacks(sq);
    attackers |= king_atk & board->pieces[color][KING];
    
    Bitboard bishop_atk = bishop_attacks(sq, occupied);
    attackers |= bishop_atk & (board->pieces[color][BISHOP] | board->pieces[color][QUEEN]);
    
    Bitboard rook_atk = rook_attacks(sq, occupied);
    attackers |= rook_atk & (board->pieces[color][ROOK] | board->pieces[color][QUEEN]);
    
    return attackers;
}


// Move Execution

void make_move(Board* board, Move move) {
    Square from = move_from(move);
    Square to = move_to(move);
    MoveFlags flags = move_flags(move);
    Color us = board->side_to_move;
    Color them = (us == WHITE) ? BLACK : WHITE;
    
    PieceType piece = piece_on(board, from);
    PieceType captured = NO_PIECE_TYPE;
    
    // Save undo information
    board->undo_stack[board->undo_index].castling_rights = board->castling_rights;
    board->undo_stack[board->undo_index].en_passant_square = board->en_passant_square;
    board->undo_stack[board->undo_index].halfmove_clock = board->halfmove_clock;
    board->undo_stack[board->undo_index].hash = board->hash;
    board->undo_stack[board->undo_index].captured_piece = NO_PIECE_TYPE;
    board->undo_index++;
    
    // Remove old castling rights from hash
    board->hash ^= castling_keys[board->castling_rights];
    
    // Remove old en passant from hash
    if (board->en_passant_square != NO_SQUARE) {
        board->hash ^= en_passant_keys[board->en_passant_square];
    }
    
    // Remove piece from origin
    board->pieces[us][piece] = clear_bit(board->pieces[us][piece], from);
    board->occupied[us] = clear_bit(board->occupied[us], from);
    board->hash ^= piece_keys[us][piece][from];
    
    // Reset en passant (will be set again if this is a double pawn push)
    board->en_passant_square = NO_SQUARE;
    
    // Handle special moves
    if (flags == CAPTURE) {
        captured = piece_on(board, to);
        board->pieces[them][captured] = clear_bit(board->pieces[them][captured], to);
        board->occupied[them] = clear_bit(board->occupied[them], to);
        board->hash ^= piece_keys[them][captured][to];
        board->undo_stack[board->undo_index - 1].captured_piece = captured;
    } else if (flags == EN_PASSANT) {
        Square captured_sq = (us == WHITE) ? (to - 8) : (to + 8);
        board->pieces[them][PAWN] = clear_bit(board->pieces[them][PAWN], captured_sq);
        board->occupied[them] = clear_bit(board->occupied[them], captured_sq);
        board->hash ^= piece_keys[them][PAWN][captured_sq];
        board->undo_stack[board->undo_index - 1].captured_piece = PAWN;
    } else if (flags == CASTLE_KINGSIDE) {
        Square rook_from = (us == WHITE) ? H1 : H8;
        Square rook_to = (us == WHITE) ? F1 : F8;
        board->pieces[us][ROOK] = clear_bit(board->pieces[us][ROOK], rook_from);
        board->pieces[us][ROOK] = set_bit(board->pieces[us][ROOK], rook_to);
        board->occupied[us] = clear_bit(board->occupied[us], rook_from);
        board->occupied[us] = set_bit(board->occupied[us], rook_to);
        board->hash ^= piece_keys[us][ROOK][rook_from];
        board->hash ^= piece_keys[us][ROOK][rook_to];
    } else if (flags == CASTLE_QUEENSIDE) {
        Square rook_from = (us == WHITE) ? A1 : A8;
        Square rook_to = (us == WHITE) ? D1 : D8;
        board->pieces[us][ROOK] = clear_bit(board->pieces[us][ROOK], rook_from);
        board->pieces[us][ROOK] = set_bit(board->pieces[us][ROOK], rook_to);
        board->occupied[us] = clear_bit(board->occupied[us], rook_from);
        board->occupied[us] = set_bit(board->occupied[us], rook_to);
        board->hash ^= piece_keys[us][ROOK][rook_from];
        board->hash ^= piece_keys[us][ROOK][rook_to];
    }
    
    // Place piece on destination (or promoted piece)
    if (is_promotion(move)) {
        PieceType promoted = promotion_piece(move);
        board->pieces[us][promoted] = set_bit(board->pieces[us][promoted], to);
        board->hash ^= piece_keys[us][promoted][to];
    } else {
        board->pieces[us][piece] = set_bit(board->pieces[us][piece], to);
        board->hash ^= piece_keys[us][piece][to];
    }
    board->occupied[us] = set_bit(board->occupied[us], to);
    
    // Update composite occupancy
    board->all_occupied = board->occupied[WHITE] | board->occupied[BLACK];
    
    // Update castling rights
    // Remove castling rights if king or rook moves
    if (piece == KING) {
        if (us == WHITE) {
            board->castling_rights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
        } else {
            board->castling_rights &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
        }
    }
    
    // Remove castling rights if rook moves or is captured
    if (from == A1 || to == A1) board->castling_rights &= ~WHITE_QUEENSIDE;
    if (from == H1 || to == H1) board->castling_rights &= ~WHITE_KINGSIDE;
    if (from == A8 || to == A8) board->castling_rights &= ~BLACK_QUEENSIDE;
    if (from == H8 || to == H8) board->castling_rights &= ~BLACK_KINGSIDE;
    
    // Add new castling rights to hash
    board->hash ^= castling_keys[board->castling_rights];
    
    // Set en passant square for double pawn push
    if (piece == PAWN && abs(square_rank(to) - square_rank(from)) == 2) {
        board->en_passant_square = (us == WHITE) ? (from + 8) : (from - 8);
        board->hash ^= en_passant_keys[board->en_passant_square];
    }
    
    // Update halfmove clock (reset on pawn move or capture)
    if (piece == PAWN || captured != NO_PIECE_TYPE) {
        board->halfmove_clock = 0;
    } else {
        board->halfmove_clock++;
    }
    
    // Update fullmove number (increment after black's move)
    if (us == BLACK) {
        board->fullmove_number++;
    }
    
    // Toggle side to move
    board->side_to_move = them;
    board->hash ^= side_key;
    
    // Update position history for repetition detection
    board->position_history[board->history_index++] = board->hash;
}

void unmake_move(Board* board, Move move) {
    Square from = move_from(move);
    Square to = move_to(move);
    MoveFlags flags = move_flags(move);
    
    // Toggle side to move back
    Color us = (board->side_to_move == WHITE) ? BLACK : WHITE;  // We moved, so switch back
    Color them = board->side_to_move;
    board->side_to_move = us;
    
    // Restore undo information
    board->undo_index--;
    UndoInfo* undo = &board->undo_stack[board->undo_index];
    board->castling_rights = undo->castling_rights;
    board->en_passant_square = undo->en_passant_square;
    board->halfmove_clock = undo->halfmove_clock;
    board->hash = undo->hash;
    PieceType captured = undo->captured_piece;
    
    // Determine piece type (handle promotions)
    PieceType piece = is_promotion(move) ? PAWN : piece_on(board, to);
    
    // Remove piece from destination
    if (is_promotion(move)) {
        PieceType promoted = promotion_piece(move);
        board->pieces[us][promoted] = clear_bit(board->pieces[us][promoted], to);
    } else {
        board->pieces[us][piece] = clear_bit(board->pieces[us][piece], to);
    }
    board->occupied[us] = clear_bit(board->occupied[us], to);
    
    // Place piece back on origin
    board->pieces[us][piece] = set_bit(board->pieces[us][piece], from);
    board->occupied[us] = set_bit(board->occupied[us], from);
    
    // Restore captured piece
    if (flags == CAPTURE) {
        board->pieces[them][captured] = set_bit(board->pieces[them][captured], to);
        board->occupied[them] = set_bit(board->occupied[them], to);
    } else if (flags == EN_PASSANT) {
        Square captured_sq = (us == WHITE) ? (to - 8) : (to + 8);
        board->pieces[them][PAWN] = set_bit(board->pieces[them][PAWN], captured_sq);
        board->occupied[them] = set_bit(board->occupied[them], captured_sq);
    } else if (flags == CASTLE_KINGSIDE) {
        Square rook_from = (us == WHITE) ? H1 : H8;
        Square rook_to = (us == WHITE) ? F1 : F8;
        board->pieces[us][ROOK] = clear_bit(board->pieces[us][ROOK], rook_to);
        board->pieces[us][ROOK] = set_bit(board->pieces[us][ROOK], rook_from);
        board->occupied[us] = clear_bit(board->occupied[us], rook_to);
        board->occupied[us] = set_bit(board->occupied[us], rook_from);
    } else if (flags == CASTLE_QUEENSIDE) {
        Square rook_from = (us == WHITE) ? A1 : A8;
        Square rook_to = (us == WHITE) ? D1 : D8;
        board->pieces[us][ROOK] = clear_bit(board->pieces[us][ROOK], rook_to);
        board->pieces[us][ROOK] = set_bit(board->pieces[us][ROOK], rook_from);
        board->occupied[us] = clear_bit(board->occupied[us], rook_to);
        board->occupied[us] = set_bit(board->occupied[us], rook_from);
    }
    
    // Update composite occupancy
    board->all_occupied = board->occupied[WHITE] | board->occupied[BLACK];
    
    // Restore position history
    board->history_index--;
    
    // Update fullmove number (decrement if we're unmaking black's move)
    if (us == BLACK) {
        board->fullmove_number--;
    }
}

bool make_move_if_legal(Board* board, Move move) {
    if (!is_legal(board, move)) {
        return false;
    }
    make_move(board, move);
    return true;
}


// Board Utilities

bool is_insufficient_material(const Board* board) {
    // K vs K
    if (board->all_occupied == (board->pieces[WHITE][KING] | board->pieces[BLACK][KING])) {
        return true;
    }
    
    // K+N vs K or K+B vs K
    int white_pieces = popcount(board->occupied[WHITE]);
    int black_pieces = popcount(board->occupied[BLACK]);
    
    if (white_pieces == 1 && black_pieces == 2) {
        if (board->pieces[BLACK][KNIGHT] || board->pieces[BLACK][BISHOP]) {
            return true;
        }
    }
    
    if (black_pieces == 1 && white_pieces == 2) {
        if (board->pieces[WHITE][KNIGHT] || board->pieces[WHITE][BISHOP]) {
            return true;
        }
    }
    
    // K+B vs K+B with same color bishops
    if (white_pieces == 2 && black_pieces == 2) {
        if (board->pieces[WHITE][BISHOP] && board->pieces[BLACK][BISHOP]) {
            Square white_bishop_sq = lsb(board->pieces[WHITE][BISHOP]);
            Square black_bishop_sq = lsb(board->pieces[BLACK][BISHOP]);
            
            // Check if bishops are on same color squares
            bool white_on_light = ((square_rank(white_bishop_sq) + square_file(white_bishop_sq)) % 2) == 0;
            bool black_on_light = ((square_rank(black_bishop_sq) + square_file(black_bishop_sq)) % 2) == 0;
            
            if (white_on_light == black_on_light) {
                return true;
            }
        }
    }
    
    return false;
}

bool is_repetition(const Board* board) {
    int count = 1;
    
    // Check history backwards (only check positions since last irreversible move)
    for (int i = board->history_index - 2; i >= 0; i--) {
        if (board->position_history[i] == board->hash) {
            count++;
            if (count >= 3) {  // Threefold repetition
                return true;
            }
        }
        
        // Stop at last irreversible move (captured piece or pawn move)
        // This is approximated by checking the halfmove clock
        if (board->history_index - 1 - i > board->halfmove_clock) {
            break;
        }
    }
    
    return false;
}

bool is_fifty_move_draw(const Board* board) {
    return board->halfmove_clock >= 100;  // 50 moves for each side
}


// Game State

GameResult get_game_result(const Board* board) {
    // Check for draws first
    if (is_fifty_move_draw(board) || is_repetition(board) || is_insufficient_material(board)) {
        return DRAW;
    }
    
    // This will be properly implemented once movegen.c is done
    // For now, return ONGOING
    return ONGOING;
}

bool is_terminal(const Board* board) {
    return get_game_result(board) != ONGOING;
}
