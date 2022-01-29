// chess.c
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "emscripten.h"

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

char *find_moves(char start_pos[]) {
    // allocate space for 42 bytes (2 chars for rank and file times 21 maximum potential moves per piece)
    movesPtr = calloc(42, sizeof(char));
    movesPtr[0] = 'e';
    movesPtr[1] = '4';

    return movesPtr;
}