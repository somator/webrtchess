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

typedef struct {
    char *piece_placement;
    char active_color;
    char *castling_availability;
    char *en_passant_target;
    int halfmove_clock;
    int fullmove_number;
} Fen;

Fen fen = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
    'w',
    "KQkq",
    "-",
    0,
    1,
};

U64 bitboards[12];

// Standard start position
void set_start_bitboards()
{
    bitboards[WHITE_KING] = 8ULL;
    bitboards[WHITE_QUEEN] = 16ULL;
    bitboards[WHITE_ROOK] = 129ULL;
    bitboards[WHITE_BISHOP] = 36ULL;
    bitboards[WHITE_KNIGHT] = 66ULL;
    bitboards[WHITE_PAWN] = 65280ULL;
    bitboards[BLACK_KING] = 576460752303423488ULL;
    bitboards[BLACK_QUEEN] = 1152921504606846976ULL;
    bitboards[BLACK_ROOK] = 9295429630892703744ULL;
    bitboards[BLACK_BISHOP] = 2594073385365405696ULL;
    bitboards[BLACK_KNIGHT] = 4755801206503243776ULL;
    bitboards[BLACK_PAWN] = 71776119061217280ULL;
}

// Calculated moves will be stored in this array of chars representating algebraic notation
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

// Convert from bitboard representation to algebraic notation and return it as a string
char *bitboard_to_an(U64 bitboard) {
    char an[2];
    U64 single_pos = 1ULL;
    for (int i = 0; i < 64; i++) {
        if (bitboard & single_pos) {
            // Determine File
            an[0] = ('h' - (i % 8));
            // Determine Rank
            an[1] = ((i / 8) + '1');
            return an;
        }
        single_pos = single_pos << 1;
    }
    return "";
}

// Convert from bitboard representation to algebraic notation (multiple moves) and store in movesPtr
void update_moves(U64 bitboard) {
    // allocate space for 42 bytes (2 chars for rank and file times 21 maximum potential moves per piece)
    movesPtr = calloc(42, sizeof(char));
    int movesPtr_index = 0;
    U64 single_pos = 1ULL;
    for (int i = 0; i < 64; i++) {
        if (bitboard & single_pos) {
            // Determine File
            movesPtr[movesPtr_index] = ('h' - (i % 8));
            movesPtr_index++;
            // Determine Rank
            movesPtr[movesPtr_index] = ((i / 8) + '1');
            movesPtr_index++;
        }
        single_pos = single_pos << 1;
    }
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

// Return a bitboard containing all of the opposition's pieces
U64 opp_bitboard(bool is_white)
{
    return my_bitboard(!is_white);
}

// Return a bitboard containing all pieces
U64 all_bitboard()
{
    U64 all_bb = 0ULL;
    int i;

    for (i = 0; i < 12; i++) {
        all_bb = all_bb | bitboards[i];
    }
    return all_bb;
}

// Return true if a square is unoccupied
bool unoccupied_square(U64 square)
{
    if ((all_bitboard() & square) == 0) {
        return true;
    } else {
        return false;
    }
}

// Return the rank of a square
int rank(U64 square)
{
    U64 bitshift_me = 1ULL;
    for (int i=0; i<64; i++) {
        if (square & bitshift_me) {
            return ((i / 8) + 1);
        }
        bitshift_me = bitshift_me << 1;
    }
    return 0;
}

// Return the file of a square
char file(U64 square)
{
    U64 bitshift_me = 1ULL;
    for (int i=0; i<64; i++) {
        if (square & bitshift_me) {
            return ('h' - (i % 8));
        }
        bitshift_me = bitshift_me << 1;
    }
    return '0';
}

// Return true if string contains a certain character
bool string_contains(char *string, char c)
{
    if (*string == c) {
        return 1;
    } else if (*string == '\0') {
        return 0;
    } else {
        return string_contains(string + 1, c);
    }
}

U64 king_pattern(U64 start_pos, bool is_white)
{
    U64 moves = 0ULL;
    U64 not_my_bb = ~my_bitboard(is_white);
    U64 possible_move;

    possible_move = (start_pos << 8) & not_my_bb & ~RANK_1;
    moves = moves | possible_move;
    possible_move = (start_pos >> 1) & not_my_bb & ~FILE_A;
    moves = moves | possible_move;
    possible_move = (start_pos >> 8) & not_my_bb & ~RANK_8;
    moves = moves | possible_move;
    possible_move = (start_pos << 1) & not_my_bb & ~FILE_H;
    moves = moves | possible_move;
    possible_move = (start_pos << 7) & not_my_bb & ~FILE_A & ~RANK_1;
    moves = moves | possible_move;
    possible_move = (start_pos >> 9) & not_my_bb & ~FILE_A & ~RANK_8;
    moves = moves | possible_move;
    possible_move = (start_pos >> 7) & not_my_bb & ~FILE_H & ~RANK_8;
    moves = moves | possible_move;
    possible_move = (start_pos << 9) & not_my_bb & ~FILE_H & ~RANK_1;
    moves = moves | possible_move;

    // Castling
    if (is_white) {
        if (string_contains(fen.castling_availability, 'K')) {
            if (unoccupied_square(2ULL) && unoccupied_square(4ULL)) {
                moves = moves | 2ULL;
            }
        }
        else if (string_contains(fen.castling_availability, 'Q')) {
            if (unoccupied_square(16ULL) && unoccupied_square(32ULL) && unoccupied_square(64ULL)) {
                moves = moves | 32ULL;
            }
        }
    }
    // Black
    else {
        if (string_contains(fen.castling_availability, 'k')) {
            if (unoccupied_square(144115188075855872ULL) && unoccupied_square(288230376151711744ULL)) {
                moves = moves | 144115188075855872ULL;
            }
        }
        else if (string_contains(fen.castling_availability, 'q')) {
            if (unoccupied_square(1152921504606846976ULL) && unoccupied_square(2305843009213693952ULL) && unoccupied_square(4611686018427387904ULL)) {
                moves = moves | 2305843009213693952ULL;
            }
        }
    }

    return moves;
}

U64 queen_pattern(U64 start_pos, bool is_white)
{
    U64 moves = 0ULL;
    U64 not_my_bb = ~my_bitboard(is_white);
    U64 possible_move;

    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 8) & not_my_bb & ~RANK_1;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 7) & not_my_bb & ~FILE_A & ~RANK_1;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 1) & not_my_bb & ~FILE_A;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 9) & not_my_bb & ~FILE_A & ~RANK_8;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 8) & not_my_bb & ~RANK_8;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 7) & not_my_bb & ~FILE_H & ~RANK_8;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 1) & not_my_bb & ~FILE_H;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 9) & not_my_bb & ~FILE_H & ~RANK_1;
        moves = moves | possible_move;
    }

    return moves;
}

U64 rook_pattern(U64 start_pos, bool is_white)
{
    U64 moves = 0ULL;
    U64 not_my_bb = ~my_bitboard(is_white);
    U64 possible_move;

    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 8) & not_my_bb & ~RANK_1;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 1) & not_my_bb & ~FILE_A;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 8) & not_my_bb & ~RANK_8;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 1) & not_my_bb & ~FILE_H;
        moves = moves | possible_move;
    }

    return moves;
}

U64 bishop_pattern(U64 start_pos, bool is_white)
{
    U64 moves = 0ULL;
    U64 not_my_bb = ~my_bitboard(is_white);
    U64 possible_move;
    
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 7) & not_my_bb & ~FILE_A & ~RANK_1;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 9) & not_my_bb & ~FILE_A & ~RANK_8;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 7) & not_my_bb & ~FILE_H & ~RANK_8;
        moves = moves | possible_move;
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 9) & not_my_bb & ~FILE_H & ~RANK_1;
        moves = moves | possible_move;
    }

    return moves;
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

U64 pawn_pattern(U64 start_pos, bool is_white)
{
    U64 moves = 0ULL;
    U64 opp_bb = opp_bitboard(is_white);
    U64 ep_target = an_to_bitboard(fen.en_passant_target);
    U64 forward_and_to_left;
    U64 forward_and_to_right;

    if (is_white) {
        if (unoccupied_square(start_pos << 8)) {
            moves = moves | (start_pos << 8);
            if (rank(start_pos) == 2 && unoccupied_square(start_pos << 16)) {
                moves = moves | (start_pos << 16);
            }
        }
        forward_and_to_left = start_pos << 9;
        forward_and_to_right = start_pos << 7;
        if ((opp_bb | ep_target) & (forward_and_to_left & ~FILE_H)) {
            moves = moves | forward_and_to_left;
        }
        else if ((opp_bb | ep_target) & (forward_and_to_right & ~FILE_A)) {
            moves = moves | forward_and_to_right;
        }
    }
    else {
        if (unoccupied_square(start_pos >> 8)) {
            moves = moves | (start_pos >> 8);
            if (rank(start_pos) == 7 && unoccupied_square(start_pos >> 16)) {
                moves = moves | (start_pos >> 16);
            }
        }
        forward_and_to_left = start_pos >> 9;
        forward_and_to_right = start_pos >> 7;
        if ((opp_bb | ep_target) & (forward_and_to_left & ~FILE_A)) {
            moves = moves | forward_and_to_left;
        }
        else if ((opp_bb | ep_target) & (forward_and_to_right & ~FILE_H)) {
            moves = moves | forward_and_to_right;
        }
    }

    return moves;
}

char *find_moves(char start_pos[]) {
    U64 moves;
    bool is_white;

    // Deallocate movesPtr, will be allocated again inside update_moves function call
    free(movesPtr);

    U64 start_pos_bb = an_to_bitboard(start_pos);
    for (int i = 0; i < 12; i++) {
        if (start_pos_bb & bitboards[i]) {
            // Determine Color
            if (i < 6) {
                is_white = true;
            } else {
                is_white = false;
            }

            // Determine Piece
            switch(i % 6) {
                // King
                case 0:
                    moves = king_pattern(start_pos_bb, is_white);
                    update_moves(moves);
                    break;
                // Queen
                case 1:
                    moves = queen_pattern(start_pos_bb, is_white);
                    update_moves(moves);
                    break;
                // Rook
                case 2:
                    moves = rook_pattern(start_pos_bb, is_white);
                    update_moves(moves);
                    break;
                // Bishop
                case 3:
                    moves = bishop_pattern(start_pos_bb, is_white);
                    update_moves(moves);
                    break;
                // Knight
                case 4:
                    moves = knight_pattern(start_pos_bb, is_white);
                    update_moves(moves);
                    break;
                // Pawn
                case 5:
                    moves = pawn_pattern(start_pos_bb, is_white);
                    update_moves(moves);
                    break;
                // Default
                default:
                    // Allocate memory to movesPtr in edge case where update_moves isn't called
                    movesPtr = calloc(42, sizeof(char));
            }
        }
    }

    return movesPtr;
}

char *make_move(char start_pos[], char end_pos[]) {
    bool is_white;
    int piece_type;
    char return_value[100] = "to do";

    U64 start_pos_bb = an_to_bitboard(start_pos);
    U64 end_pos_bb = an_to_bitboard(start_pos);

    U64 start_pos_bb = an_to_bitboard(start_pos);
    for (int i = 0; i < 12; i++) {
        if (start_pos_bb & bitboards[i]) {
            // Determine Color
            if (i < 6) {
                is_white = true;
            } else {
                is_white = false;
            }
            // Determine Piece Type
            piece_type = i % 6;

            // Queen, Rook, Bishop, and Knight
            if (piece_type>0 && piece_type<5) {
                // And the bitwise complement of our start position to our bitboard to remove it from start position
                bitboards[i] = bitboards[i] & ~start_pos_bb;
                // And the bitwise complement of our end position to each bitboard to remove any captured piece
                for (int j = 0; j < 12; j++) {
                    bitboards[j] = bitboards[j] & ~end_pos_bb;
                }
                // Or the end position to our bitboard to add it to the end position
                bitboards[i] = bitboards[i] | end_pos_bb;
            }
            // King
            else if (piece_type == 0) {
                // Castling
                // King side castling
                if (start_pos_bb >> 2 == end_pos_bb) {
                    // Remove rook
                    bitboards[i+2] = bitboards[i+2] & ~(end_pos_bb >> 1);
                    // Add rook
                    bitboards[i+2] = bitboards[i+2] | (end_pos_bb << 1);
                }
                // Queen side castling
                else if (start_pos_bb << 2 == end_pos_bb) {
                    // Remove rook
                    bitboards[i+2] = bitboards[i+2] & ~(end_pos_bb << 2);
                    // Add rook
                    bitboards[i+2] = bitboards[i+2] | (end_pos_bb >> 1);
                }
                else {
                    // And the bitwise complement of our start position to our bitboard to remove it from start position
                    bitboards[i] = bitboards[i] & ~start_pos_bb;
                    // And the bitwise complement of our end position to each bitboard to remove any captured piece
                    for (int j = 0; j < 12; j++) {
                        bitboards[j] = bitboards[j] & ~end_pos_bb;
                    }
                    // Or the end position to our bitboard to add it to the end position
                    bitboards[i] = bitboards[i] | end_pos_bb;
                }
            }
            // Pawn
            else if (piece_type == 5) {
                if (file(start_pos_bb) == file(end_pos_bb)) {
                    // And the bitwise complement of our start position to our bitboard to remove it from start position
                    bitboards[i] = bitboards[i] & ~start_pos_bb;
                    // Or the end position to our bitboard to add it to the end position
                    bitboards[i] = bitboards[i] | end_pos_bb;
                }
            }
        }
    }

    return return_value;
}