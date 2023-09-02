// chess.c
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
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

// Standard start position
void set_start_bitboards()
{
    bitboards[WHITE_KING] = (U64)8;
    bitboards[WHITE_QUEEN] = (U64)16;
    bitboards[WHITE_ROOK] = (U64)129;
    bitboards[WHITE_BISHOP] = (U64)36;
    bitboards[WHITE_KNIGHT] = (U64)66;
    bitboards[WHITE_PAWN] = (U64)65280;
    bitboards[BLACK_KING] = (U64)576460752303423488;
    bitboards[BLACK_QUEEN] = (U64)1152921504606846976;
    bitboards[BLACK_ROOK] = (U64)9295429630892703744;
    bitboards[BLACK_BISHOP] = (U64)2594073385365405696;
    bitboards[BLACK_KNIGHT] = (U64)4755801206503243776;
    bitboards[BLACK_PAWN] = (U64)71776119061217280;
}

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

// Return a bitboard containing all of one side's pieces
U64 my_bitboard(bool is_white)
{
    U64 my_bb = 0ULL;
    int i = 0;
    int max_i;
    if (!is_white) {
        i = i + 6;
    }
    max_i = i + 6;
    for ( ; i < max_i; i++) {
        my_bb = my_bb | bitboards[i];
    }
    return my_bb;
}

U64 knight_pattern(U64 start_pos, bool is_white)
{
    U64 moves = 0ULL;
    U64 not_my_bb = ~my_bitboard(is_white);

    moves = moves | ((start_pos << 15) & not_my_bb & ~FILE_A);
    moves = moves | ((start_pos <<  6) & not_my_bb & ~FILE_A & ~FILE_B);
    moves = moves | ((start_pos >> 10) & not_my_bb & ~FILE_A & ~FILE_B);
    moves = moves | ((start_pos >> 17) & not_my_bb & ~FILE_A);
    moves = moves | ((start_pos >> 15) & not_my_bb & ~FILE_H);
    moves = moves | ((start_pos >>  6) & not_my_bb & ~FILE_H & ~FILE_G);
    moves = moves | ((start_pos << 10) & not_my_bb & ~FILE_H & ~FILE_G);
    moves = moves | ((start_pos << 17) & not_my_bb & ~FILE_H);

    return moves;
}

char *find_moves(char start_pos[]) {

    printf("lets play chess!\n");

    // allocate space for 42 bytes (2 chars for rank and file times 21 maximum potential moves per piece)
    movesPtr = calloc(42, sizeof(char));
    movesPtr[0] = 'e';
    movesPtr[1] = '4';
    movesPtr[2] = 'e';
    movesPtr[3] = '5';

    U64 start_pos_bb = an_to_bitboard(start_pos);
    for (int i = 0; i < 12; i++) {
        if (start_pos_bb & bitboards[i]) {
            ;
        }
    }

    return movesPtr;
}