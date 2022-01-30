// chess.c
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "emscripten.h"

/* 8x8 bitboards are stored as 64 bit unsigned integers.
files correspond to columns, and ranks correspond to rows */
#define FILE_A 0x8080808080808080ULL
#define FILE_B 0x4040404040404040ULL
#define FILE_C 0x2020202020202020ULL
#define FILE_D 0x1010101010101010ULL
#define FILE_E 0x0808080808080808ULL
#define FILE_F 0x0404040404040404ULL
#define FILE_G 0x0202020202020202ULL
#define FILE_H 0x0101010101010101ULL

#define RANK_1 0x00000000000000ffULL
#define RANK_2 0x000000000000ff00ULL
#define RANK_3 0x0000000000ff0000ULL
#define RANK_4 0x00000000ff000000ULL
#define RANK_5 0x000000ff00000000ULL
#define RANK_6 0x0000ff0000000000ULL
#define RANK_7 0x00ff000000000000ULL
#define RANK_8 0xff00000000000000ULL

typedef unsigned long long U64;

enum Piece_Type {
    WHITE_KING = 0,
    WHITE_QUEEN = 1,
    WHITE_ROOK = 2,
    WHITE_BISHOP = 3,
    WHITE_KNIGHT = 4,
    WHITE_PAWN = 5,
    BLACK_KING = 6,
    BLACK_QUEEN = 7,
    BLACK_ROOK = 8,
    BLACK_BISHOP = 9,
    BLACK_KNIGHT = 10,
    BLACK_PAWN = 11,
};

U64 bitboards[12];

char *movesPtr;

// Convert from algebraic notation to bitboard representation
U64 an_to_bitboard(char *an)
{
    char file, rank;
    U64 bitboard;

    file = an[0];
    rank = an[1];
    bitboard = 1ULL;

    bitboard = bitboard << (104 - file);
    bitboard = bitboard << (8 * (rank - 49));
    return bitboard;
}

char *find_moves(char start_pos[]) {
    // allocate space for 42 bytes (2 chars for rank and file times 21 maximum potential moves per piece)
    movesPtr = calloc(42, sizeof(char));
    movesPtr[0] = 'e';
    movesPtr[1] = '4';

    U64 start_pos_bb = an_to_bitboard(start_pos);
    for (int i = 0; i < 12; i++) {
        ;
    }

    return movesPtr;
}