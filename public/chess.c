// chess.c
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "emscripten.h"

typedef unsigned long long U64;

enum Color {
    WHITE = 0,
    BLACK = 1,
};

enum Type {
    PAWN = 0,
    KNIGHT = 1,
    BISHOP = 2,
    ROOK = 3,
    QUEEN = 4,
    KING = 5,
};

char *movesPtr;

char *find_moves(char start_pos[]) {
    // allocate space for 42 bytes (2 chars for rank and file times 21 maximum potential moves per piece)
    movesPtr = calloc(42, sizeof(char));
    movesPtr[0] = 'e';
    movesPtr[1] = '4';

    return movesPtr;
}