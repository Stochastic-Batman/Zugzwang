#include "bitboard.h"
#include "board.h"
#include "evaluation.h"
#include "moves.h"
#include "movegen.h"
#include "notation.h"
#include "search.h"
#include "transposition.h"
#include "zobrist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print_game_rules(void) {
    printf("\n");
    printf("===============================================\n");
    printf("              ZUGZWANG CHESS ENGINE           \n");
    printf("===============================================\n");
    printf("\n");
    printf("GAME MECHANICS:\n");
    printf("  - You will play against the engine\n");
    printf("  - Choose your color (White or Black)\n");
    printf("  - Moves are displayed in a table:\n");
    printf("      Move | White            | Black\n");
    printf("    ---------------------------------------\n");
    printf("      1.   | [White's move]   | [Black's move]\n");
    printf("      2.   | [White's move]   | [Black's move]\n");
    printf("      ...\n");
    printf("\n");
    printf("INPUT RULES:\n");
    printf("  - When it's YOUR turn, enter your move\n");
    printf("  - When it's the ENGINE's turn, it plays automatically\n");
    printf("  - Use standard algebraic notation:\n");
    printf("      e4, Nf3, Bxc6, O-O, e8=Q, etc.\n");
    printf("\n");
    printf("COMMANDS:\n");
    printf("  moves - Show all legal moves\n");
    printf("  help  - Display notation help\n");
    printf("  quit  - Exit the game\n");
    printf("\n");
    printf("===============================================\n");
    printf("\n");
}

void play_game(Color player_color, int search_depth) {
    Board board;
    init_board(&board);
    
    TranspositionTable tt;
    init_tt(&tt, 64);
    
    SearchParams params = {
        .max_depth = search_depth,
        .tt = &tt
    };
    
    SearchInfo info;
    
    printf("\n");
    printf("   White        Black\n");
    printf("------------------------\n");
    
    int move_number = 1;
    
    while (true) {
        MoveList legal_moves;
        generate_moves(&board, &legal_moves);
        
        int legal_count = 0;
        for (int i = 0; i < legal_moves.count; i++) {
            if (is_legal(&board, legal_moves.moves[i])) {
                legal_count++;
                break;
            }
        }
        
        if (legal_count == 0) {
            printf("\n");
            if (is_in_check(&board, board.side_to_move)) {
                if (board.side_to_move == WHITE) {
                    printf("Checkmate! Black wins.\n");
                } else {
                    printf("Checkmate! White wins.\n");
                }
            } else {
                printf("Stalemate! Game is a draw.\n");
            }
            break;
        }
        
        if (is_fifty_move_draw(&board)) {
            printf("\nDraw by fifty-move rule.\n");
            break;
        }
        
        if (is_repetition(&board)) {
            printf("\nDraw by threefold repetition.\n");
            break;
        }
        
        Move move = 0;
        char move_str[16] = "";
        
        if (board.side_to_move == player_color) {
            if (board.side_to_move == WHITE) {
                printf("%2d. ", move_number);
            } else {
                printf("    ");
            }
           
            move = read_user_move(&board);

            if (move == 0) {
                printf("\nGame terminated.\n");
                break;
            }

            move_to_algebraic(&board, move, move_str);
        } else {
            if (board.side_to_move == WHITE) {
                printf("%2d. ", move_number);
            } else {
                printf("    ");
            }
            
            memset(&info, 0, sizeof(SearchInfo));
            move = find_best_move(&board, search_depth, &info, &params);
            
            if (move == 0) {
                printf("Engine error: no move found.\n");
                break;
            }
            
            move_to_algebraic(&board, move, move_str);
            printf("%-6s", move_str);
        }
        
        make_move(&board, move);
        
        if (board.side_to_move == WHITE) {
            if (player_color != BLACK) printf("\n");
            move_number++;
        } else {
            if (player_color == WHITE) {
                printf("\033[1A\r%2d. %-6s   ", move_number, move_str);
            } else {
                printf("   ");
            }
        }
    }
    
    free_tt(&tt);
}

int main(void) {
    init_bitboards();
    init_zobrist();
    
    print_game_rules();
    
    Color player_color = choose_player_color();
    
    if (player_color == WHITE) {
        printf("You are playing as White.\n");
    } else {
        printf("You are playing as Black.\n");
    }
    
    printf("Starting game...\n");
    
    play_game(player_color, 5);
    
    printf("\nThank you for playing!\n");
    
    return 0;
}
