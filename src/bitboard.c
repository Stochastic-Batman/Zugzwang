#include "bitboard.h"
#include <string.h>

#define FILE_A 0x0101010101010101ULL
#define FILE_H 0x8080808080808080ULL


// Basic Bitboard Operations

Bitboard set_bit(Bitboard bb, Square sq) {
    return (bb | (1ULL << sq));
}

Bitboard clear_bit(Bitboard bb, Square sq) {
    return (bb & ~(1ULL << sq));
}

bool get_bit(Bitboard bb, Square sq) {
    return (bb >> sq) & 1ULL;    
}

int popcount(Bitboard bb) {
    return __builtin_popcountll(bb);
}

Square lsb(Bitboard bb) {
    if (bb == 0) return NO_SQUARE;
    // __builtin_ctzll counts trailing zeros, which equals the index of the LSB
    return (Square)__builtin_ctzll(bb);
}

Square pop_lsb(Bitboard* bb) {
    Square idx = lsb(*bb);
    *bb &= (*bb - 1);
    return idx;
}


// Square Utilities

int square_rank(Square sq) {
    return sq >> 3;
}

int square_file(Square sq) {
    return sq & 7;
}
    
Square make_square(int rank, int file) {
    return (Square)((rank << 3) | file);
}

Bitboard square_bb(Square sq) {
    return 1ULL << sq;
}


// Masks

Bitboard rank_mask(int rank) {
    return 0xFFULL << (rank * 8);
}

Bitboard file_mask(int file) {
    return 0x0101010101010101ULL << file;
}

Bitboard diagonal_mask(Square sq) {
    Bitboard mask = 0ULL;
    int diag = (sq >> 3) - (sq & 7); // (rank - file)
    
    for (int i = 0; i < 64; i++) {
        if ((i >> 3) - (i & 7) == diag) {
            mask |= (1ULL << i);
       }
    }

    return mask;
}

Bitboard anti_diagonal_mask(Square sq) {
    Bitboard mask = 0ULL;
    int anti_diag = (sq >> 3) + (sq & 7); // (rank + file)
    
    for (int i = 0; i < 64; i++) {
        if ((i >> 3) + (i & 7) == anti_diag) {
            mask |= (1ULL << i);
        }
    }

    return mask;
}


// Non-Sliding Piece Attack Tables

Bitboard pawn_attack_table[2][64];
Bitboard knight_attack_table[64];
Bitboard king_attack_table[64];

Bitboard pawn_attacks(Square sq, Color color) {
    return pawn_attack_table[color][sq];
}

Bitboard knight_attacks(Square sq) {
    return knight_attack_table[sq];
}

Bitboard king_attacks(Square sq) {
    return king_attack_table[sq];
}


// Magic Bitboards - Tables and Constants

// Magic numbers (pre-computed for each square)
// These are known good magic numbers from chess programming literature
Bitboard rook_magics[64] = {
    0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
    0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
    0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
    0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
    0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010000800ULL, 0x0000808008000400ULL,
    0x0000808004000200ULL, 0x0000808002000100ULL, 0x0000010100020004ULL, 0x0000020000408104ULL,
    0x0000208080004000ULL, 0x0000200040005000ULL, 0x0000100080200080ULL, 0x0000080080100080ULL,
    0x0000040080080080ULL, 0x0000020080040080ULL, 0x0000010080800200ULL, 0x0000800080004100ULL,
    0x0000204000800080ULL, 0x0000200040401000ULL, 0x0000100080802000ULL, 0x0000080080801000ULL,
    0x0000040080800800ULL, 0x0000020080800400ULL, 0x0000020001010004ULL, 0x0000800040800100ULL,
    0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000004081020004ULL,
    0x0000204000800080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000800100020080ULL, 0x0000800041000080ULL,
    0x00FFFCDDFCED714AULL, 0x007FFCDDFCED714AULL, 0x003FFFCDFFD88096ULL, 0x0000040810002101ULL,
    0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001FFFAABFAD1A2ULL
};

Bitboard bishop_magics[64] = {
    0x0002020202020200ULL, 0x0002020202020000ULL, 0x0004010202000000ULL, 0x0004040080000000ULL,
    0x0001104000000000ULL, 0x0000821040000000ULL, 0x0000410410400000ULL, 0x0000104104104000ULL,
    0x0000040404040400ULL, 0x0000020202020200ULL, 0x0000040102020000ULL, 0x0000040400800000ULL,
    0x0000011040000000ULL, 0x0000008210400000ULL, 0x0000004104104000ULL, 0x0000002082082000ULL,
    0x0004000808080800ULL, 0x0002000404040400ULL, 0x0001000202020200ULL, 0x0000800802004000ULL,
    0x0000800400A00000ULL, 0x0000200100884000ULL, 0x0000400082082000ULL, 0x0000200041041000ULL,
    0x0002080010101000ULL, 0x0001040008080800ULL, 0x0000208004010400ULL, 0x0000404004010200ULL,
    0x0000840000802000ULL, 0x0000404002011000ULL, 0x0000808001041000ULL, 0x0000404000820800ULL,
    0x0001041000202000ULL, 0x0000820800101000ULL, 0x0000104400080800ULL, 0x0000020080080080ULL,
    0x0000404040040100ULL, 0x0000808100020100ULL, 0x0001010100020800ULL, 0x0000808080010400ULL,
    0x0000820820004000ULL, 0x0000410410002000ULL, 0x0000082088001000ULL, 0x0000002011000800ULL,
    0x0000080100400400ULL, 0x0001010101000200ULL, 0x0002020202000400ULL, 0x0001010101000200ULL,
    0x0000410410400000ULL, 0x0000208208200000ULL, 0x0000002084100000ULL, 0x0000000020880000ULL,
    0x0000001002020000ULL, 0x0000040408020000ULL, 0x0004040404040000ULL, 0x0002020202020000ULL,
    0x0000104104104000ULL, 0x0000002082082000ULL, 0x0000000020841000ULL, 0x0000000000208800ULL,
    0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL
};

// Occupancy masks (exclude edges for magic hashing)
Bitboard rook_masks[64];
Bitboard bishop_masks[64];

// Shift amounts for magic multiplication
int rook_shifts[64];
int bishop_shifts[64];

// Attack lookup tables
Bitboard rook_attacks_table[64][4096];
Bitboard bishop_attacks_table[64][512];


// Helper Functions for Magic Bitboard Initialization

static Bitboard generate_rook_mask(Square sq) {
    Bitboard mask = 0ULL;
    int r = sq >> 3;
    int f = sq & 7;

    // North (exclude rank 7)
    for (int i = r + 1; i < 7; i++) {
        mask |= (1ULL << (i * 8 + f));
    }

    // South (exclude rank 0)
    for (int i = r - 1; i > 0; i--) {
        mask |= (1ULL << (i * 8 + f));
    }

    // East (exclude file h)
    for (int i = f + 1; i < 7; i++) {
        mask |= (1ULL << (r * 8 + i));
    }

    // West (exclude file a)
    for (int i = f - 1; i > 0; i--) {
        mask |= (1ULL << (r * 8 + i));
    }

    return mask;
}

static Bitboard generate_bishop_mask(Square sq) {
    Bitboard mask = 0ULL;
    int r = sq >> 3;
    int f = sq & 7;

    // NE
    for (int i = 1; r + i < 7 && f + i < 7; i++) {
        mask |= (1ULL << ((r + i) * 8 + (f + i)));
    }

    // NW
    for (int i = 1; r + i < 7 && f - i > 0; i++) {
        mask |= (1ULL << ((r + i) * 8 + (f - i)));
    }

    // SE
    for (int i = 1; r - i > 0 && f + i < 7; i++) {
        mask |= (1ULL << ((r - i) * 8 + (f + i)));
    }

    // SW
    for (int i = 1; r - i > 0 && f - i > 0; i++) {
        mask |= (1ULL << ((r - i) * 8 + (f - i)));
    }

    return mask;
}

static Bitboard generate_rook_attacks_slow(Square sq, Bitboard occupied) {
    Bitboard attacks = 0ULL;
    int r = sq >> 3;
    int f = sq & 7;

    // North
    for (int i = r + 1; i <= 7; i++) {
        attacks |= (1ULL << (i * 8 + f));
        if (occupied & (1ULL << (i * 8 + f))) break;
    }

    // South
    for (int i = r - 1; i >= 0; i--) {
        attacks |= (1ULL << (i * 8 + f));
        if (occupied & (1ULL << (i * 8 + f))) break;
    }

    // East
    for (int i = f + 1; i <= 7; i++) {
        attacks |= (1ULL << (r * 8 + i));
        if (occupied & (1ULL << (r * 8 + i))) break;
    }

    // West
    for (int i = f - 1; i >= 0; i--) {
        attacks |= (1ULL << (r * 8 + i));
        if (occupied & (1ULL << (r * 8 + i))) break;
    }

    return attacks;
}

static Bitboard generate_bishop_attacks_slow(Square sq, Bitboard occupied) {
    Bitboard attacks = 0ULL;
    int r = sq >> 3;
    int f = sq & 7;

    // NE
    for (int i = 1; r + i <= 7 && f + i <= 7; i++) {
        attacks |= (1ULL << ((r + i) * 8 + (f + i)));
        if (occupied & (1ULL << ((r + i) * 8 + (f + i)))) break;
    }

    // NW
    for (int i = 1; r + i <= 7 && f - i >= 0; i++) {
        attacks |= (1ULL << ((r + i) * 8 + (f - i)));
        if (occupied & (1ULL << ((r + i) * 8 + (f - i)))) break;
    }

    // SE
    for (int i = 1; r - i >= 0 && f + i <= 7; i++) {
        attacks |= (1ULL << ((r - i) * 8 + (f + i)));
        if (occupied & (1ULL << ((r - i) * 8 + (f + i)))) break;
    }

    // SW
    for (int i = 1; r - i >= 0 && f - i >= 0; i++) {
        attacks |= (1ULL << ((r - i) * 8 + (f - i)));
        if (occupied & (1ULL << ((r - i) * 8 + (f - i)))) break;
    }

    return attacks;
}

// Generate all occupancy variations for a given mask
static void generate_occupancies(Bitboard mask, Bitboard* occupancies, int* count) {
    int bits[64];
    int num_bits = 0;

    // Extract bit positions
    Bitboard m = mask;
    while (m) {
        bits[num_bits++] = lsb(m);
        m &= m - 1;
    }

    // Generate all 2^num_bits combinations
    *count = 1 << num_bits;
    for (int i = 0; i < *count; i++) {
        Bitboard occ = 0ULL;
        for (int j = 0; j < num_bits; j++) {
            if (i & (1 << j)) {
                occ |= (1ULL << bits[j]);
            }
        }
        occupancies[i] = occ;
    }
}


// Magic Bitboard Attack Functions (O(1) Lookup)

Bitboard bishop_attacks(Square sq, Bitboard occupied) {
    Bitboard relevant = occupied & bishop_masks[sq];  // Apply mask to get relevant occupancy

    // Magic multiplication and shift to get table index
    int index = (relevant * bishop_magics[sq]) >> bishop_shifts[sq];

    return bishop_attacks_table[sq][index];
}

Bitboard rook_attacks(Square sq, Bitboard occupied) {
    Bitboard relevant = occupied & rook_masks[sq];
    int index = (relevant * rook_magics[sq]) >> rook_shifts[sq];
    return rook_attacks_table[sq][index];
}

Bitboard queen_attacks(Square sq, Bitboard occupied) {
    return rook_attacks(sq, occupied) | bishop_attacks(sq, occupied);
}


// Initialization

void init_bitboards(void) {
    // Clear all tables
    memset(pawn_attack_table, 0, sizeof(pawn_attack_table));
    memset(knight_attack_table, 0, sizeof(knight_attack_table));
    memset(king_attack_table, 0, sizeof(king_attack_table));
    memset(rook_attacks_table, 0, sizeof(rook_attacks_table));
    memset(bishop_attacks_table, 0, sizeof(bishop_attacks_table));

    // Initialize non-sliding piece attacks
    for (int sq = 0; sq < 64; sq++) {
        int r = sq >> 3;
        int f = sq & 7;

        // Knight attacks
        int dr[8] = {-2, -2, -1, -1, 1, 1, 2, 2};
        int df[8] = {-1, 1, -2, 2, -2, 2, -1, 1};
        for (int i = 0; i < 8; i++) {
            int nr = r + dr[i], nf = f + df[i];
            if (nr >= 0 && nr <= 7 && nf >= 0 && nf <= 7) {
                knight_attack_table[sq] |= (1ULL << ((nr << 3) + nf));
            }
        }

        // King attacks
        for (int dr = -1; dr <= 1; dr++) {
            for (int df = -1; df <= 1; df++) {
                if (dr == 0 && df == 0) continue;
                int nr = r + dr, nf = f + df;
                if (nr >= 0 && nr <= 7 && nf >= 0 && nf <= 7) {
                    king_attack_table[sq] |= (1ULL << ((nr << 3) + nf));
                }
            }
        }

        // Pawn attacks
        if (r < 7) { // White (cannot attack from rank 8)
            if (f > 0) pawn_attack_table[WHITE][sq] |= (1ULL << (sq + 7));
            if (f < 7) pawn_attack_table[WHITE][sq] |= (1ULL << (sq + 9));
        }

        if (r > 0) { // Black (cannot attack from rank 1)
            if (f > 0) pawn_attack_table[BLACK][sq] |= (1ULL << (sq - 9));
            if (f < 7) pawn_attack_table[BLACK][sq] |= (1ULL << (sq - 7));
        }
    }

    // Initialize magic bitboards for sliding pieces
    for (int sq = 0; sq < 64; sq++) {
        // Generate masks
        rook_masks[sq] = generate_rook_mask(sq);
        bishop_masks[sq] = generate_bishop_mask(sq);

        // Calculate shift amounts (64 - number of relevant bits)
        rook_shifts[sq] = 64 - popcount(rook_masks[sq]);
        bishop_shifts[sq] = 64 - popcount(bishop_masks[sq]);

        // Generate all occupancy variations and fill attack tables
        Bitboard rook_occupancies[4096];
        int rook_count;
        generate_occupancies(rook_masks[sq], rook_occupancies, &rook_count);

        for (int i = 0; i < rook_count; i++) {
            Bitboard occ = rook_occupancies[i];
            int index = (occ * rook_magics[sq]) >> rook_shifts[sq];
            rook_attacks_table[sq][index] = generate_rook_attacks_slow(sq, occ);
        }

        Bitboard bishop_occupancies[512];
        int bishop_count;
        generate_occupancies(bishop_masks[sq], bishop_occupancies, &bishop_count);

        for (int i = 0; i < bishop_count; i++) {
            Bitboard occ = bishop_occupancies[i];
            int index = (occ * bishop_magics[sq]) >> bishop_shifts[sq];
            bishop_attacks_table[sq][index] = generate_bishop_attacks_slow(sq, occ);
        }
    }
}
