// chess.c
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "emscripten.h"

/* 8x8 bitboards are stored as 64 bit unsigned integers.
files correspond to columns, and ranks correspond to rows. */
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

// A struct representing the Forsyth–Edwards Notation (FEN) of the board state
typedef struct {
    char piece_placement[74];
    char active_color;
    char castling_availability[5];
    char en_passant_target[3];
    int halfmove_clock;
    int fullmove_number;
} Fen;

// Set default starting fen 
Fen fen = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
    'w',
    "KQkq",
    "-",
    0,
    1,
};

/* Pieces in Forsyth-Edwards notation are represented by the characters in this array,
which we can index with the Piece_Type enum */
char fen_lookup[13] = "KQRBNPkqrbnp";

// A global array of 12 bitboards representing piece placement
U64 bitboards[12];

// Calculated moves at depth 0 will be stored in this char array representating them in algebraic notation
char primary_moves_arr[43];

// Calculated moves at depth 1 will be stored in this char array representating them in algebraic notation
char secondary_moves_arr[43];

// Function prototypes to avoid implicit declarations
char *find_moves(char start_pos[], U64* bitboards_ptr, bool check_for_checks);
void process_move(char start_pos[], char end_pos[], U64 bitboards_arr[], Fen* fen_ptr);
void update_piece_placement();

// Print a single bitboard
void print_bitboard(U64 bitboard) {
    U64 bit = 1ULL << 63;
    for (int i=0; i<64; i++) {
        if (bit & bitboard) {
            printf("1");
        } else {
            printf("0");
        }
        if (i % 8 == 7) {
            printf("\n");
        }
        bit = bit >> 1;
    }
    printf("\n");
}

// Print the full board
void print_board() {
    U64 bit = 1ULL << 63;
    char c;
    for (int i=0; i<64; i++) {
        c = '-';
        for (int j=0; j<12; j++) {
            if (bit & bitboards[j]) {
                c = fen_lookup[j];
            }
        }
        printf("%c", c);
        if (i % 8 == 7) {
            printf("\n");
        }
        bit = bit >> 1;
    }
    printf("\n");
}

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

// Convert from bitboard representation to algebraic notation and return it as a pointer
char *bitboard_to_an(U64 bitboard) {
    static char an[3];
    U64 single_pos = 1ULL;
    for (int i = 0; i < 64; i++) {
        if (bitboard & single_pos) {
            // Determine File
            an[0] = ('h' - (i % 8));
            // Determine Rank
            an[1] = ((i / 8) + '1');
            an[2] = '\0';
            return an;
        }
        single_pos = single_pos << 1;
    }
    return "";
}

// Return a bitboard containing all of one side's pieces
U64 my_bitboard(bool is_white, U64* bitboards_ptr)
{
    U64 my_bb = 0ULL;
    int i = 0;
    int max_i;
    if (!is_white) {
        i = i + 6;
    }
    max_i = i + 6;
    for ( ; i < max_i; i++) {
        my_bb = my_bb | bitboards_ptr[i];
    }
    return my_bb;
}

// Return a bitboard containing all of the opposition's pieces
U64 opp_bitboard(bool is_white, U64* bitboards_ptr)
{
    return my_bitboard(!is_white, bitboards_ptr);
}

// Return a bitboard containing all pieces
U64 all_bitboard(U64* bitboards_ptr)
{
    U64 all_bb = 0ULL;
    int i;

    for (i = 0; i < 12; i++) {
        all_bb = all_bb | bitboards_ptr[i];
    }
    return all_bb;
}

// Return true if a square is unoccupied
bool unoccupied_square(U64 square, U64* bitboards_ptr)
{
    if ((all_bitboard(bitboards_ptr) & square) == 0) {
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

// Remove all chars of a certain type from a string
void remove_chars(char *s, char c)
{
    int writer = 0, reader = 0;
    while (s[reader]) {
        if (s[reader] != c) {   
            s[writer++] = s[reader];
        }
        reader++;       
    }
    s[writer]=0;
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

// Return a pointer to a string representing the entirety of the fen struct
char *stringify_fen()
{
    static char result[100];
    sprintf(result, "%s %c %s %s %i %i", fen.piece_placement, fen.active_color, fen.castling_availability, fen.en_passant_target, fen.halfmove_clock, fen.fullmove_number);
    return result;
}

/* Standard start position. These "magic numbers" are all precalculated bitboards
representing where you would expect to find each piece at the beginning of the game. */
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

/* Given a start position, a color, and the current piece placement, return 
a bitboard representing all pseudo-legal king moves. */
U64 king_pattern(U64 start_pos, bool is_white, U64* bitboards_ptr)
{
    U64 moves = 0ULL;
    U64 not_my_bb = ~my_bitboard(is_white, bitboards_ptr);
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
            if (unoccupied_square(2ULL, bitboards_ptr) && unoccupied_square(4ULL, bitboards_ptr)) {
                moves = moves | 2ULL;
            }
        }
        if (string_contains(fen.castling_availability, 'Q')) {
            if (unoccupied_square(16ULL, bitboards_ptr) && unoccupied_square(32ULL, bitboards_ptr) && unoccupied_square(64ULL, bitboards_ptr)) {
                moves = moves | 32ULL;
            }
        }
    }
    // Black
    else {
        if (string_contains(fen.castling_availability, 'k')) {
            if (unoccupied_square(144115188075855872ULL, bitboards_ptr) && unoccupied_square(288230376151711744ULL, bitboards_ptr)) {
                moves = moves | 144115188075855872ULL;
            }
        }
        if (string_contains(fen.castling_availability, 'q')) {
            if (unoccupied_square(1152921504606846976ULL, bitboards_ptr) && unoccupied_square(2305843009213693952ULL, bitboards_ptr) && unoccupied_square(4611686018427387904ULL, bitboards_ptr)) {
                moves = moves | 2305843009213693952ULL;
            }
        }
    }

    return moves;
}

/* Given a start position, a color, and the current piece placement, return 
a bitboard representing all pseudo-legal queen moves. */
U64 queen_pattern(U64 start_pos, bool is_white, U64* bitboards_ptr)
{
    U64 moves = 0ULL;
    U64 not_my_bb = ~my_bitboard(is_white, bitboards_ptr);
    U64 opp_bb = opp_bitboard(is_white, bitboards_ptr);
    U64 possible_move;

    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 8) & not_my_bb & ~RANK_1;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 7) & not_my_bb & ~FILE_A & ~RANK_1;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 1) & not_my_bb & ~FILE_A;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 9) & not_my_bb & ~FILE_A & ~RANK_8;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 8) & not_my_bb & ~RANK_8;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 7) & not_my_bb & ~FILE_H & ~RANK_8;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 1) & not_my_bb & ~FILE_H;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 9) & not_my_bb & ~FILE_H & ~RANK_1;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }

    return moves;
}

/* Given a start position, a color, and the current piece placement, return 
a bitboard representing all pseudo-legal rook moves. */
U64 rook_pattern(U64 start_pos, bool is_white, U64* bitboards_ptr)
{
    U64 moves = 0ULL;
    U64 not_my_bb = ~my_bitboard(is_white, bitboards_ptr);
    U64 opp_bb = opp_bitboard(is_white, bitboards_ptr);
    U64 possible_move;

    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 8) & not_my_bb & ~RANK_1;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 1) & not_my_bb & ~FILE_A;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 8) & not_my_bb & ~RANK_8;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 1) & not_my_bb & ~FILE_H;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }

    return moves;
}

/* Given a start position, a color, and the current piece placement, return 
a bitboard representing all pseudo-legal bishop moves. */
U64 bishop_pattern(U64 start_pos, bool is_white, U64* bitboards_ptr)
{
    U64 moves = 0ULL;
    U64 not_my_bb = ~my_bitboard(is_white, bitboards_ptr);
    U64 opp_bb = opp_bitboard(is_white, bitboards_ptr);
    U64 possible_move;
    
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 7) & not_my_bb & ~FILE_A & ~RANK_1;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 9) & not_my_bb & ~FILE_A & ~RANK_8;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move >> 7) & not_my_bb & ~FILE_H & ~RANK_8;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }
    possible_move = start_pos;
    while (possible_move) {
        possible_move = (possible_move << 9) & not_my_bb & ~FILE_H & ~RANK_1;
        moves = moves | possible_move;
        if (possible_move & opp_bb) {
            break;
        }
    }

    return moves;
}

/* Given a start position, a color, and the current piece placement, return 
a bitboard representing all pseudo-legal knight moves. */
U64 knight_pattern(U64 start_pos, bool is_white, U64* bitboards_ptr)
{
    U64 moves = 0ULL;
    U64 not_my_bb = ~my_bitboard(is_white, bitboards_ptr);

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

/* Given a start position, a color, and the current piece placement, return 
a bitboard representing all pseudo-legal king moves. */
U64 pawn_pattern(U64 start_pos, bool is_white, U64* bitboards_ptr)
{
    U64 moves = 0ULL;
    U64 opp_bb = opp_bitboard(is_white, bitboards_ptr);
    U64 ep_target = an_to_bitboard(fen.en_passant_target);
    U64 forward_and_to_left;
    U64 forward_and_to_right;

    if (is_white) {
        if (unoccupied_square(start_pos << 8, bitboards_ptr)) {
            moves = moves | (start_pos << 8);
            if (rank(start_pos) == 2 && unoccupied_square(start_pos << 16, bitboards_ptr)) {
                moves = moves | (start_pos << 16);
            }
        }
        forward_and_to_left = start_pos << 9;
        forward_and_to_right = start_pos << 7;
        if ((opp_bb | ep_target) & (forward_and_to_left & ~FILE_H)) {
            moves = moves | forward_and_to_left;
        }
        if ((opp_bb | ep_target) & (forward_and_to_right & ~FILE_A)) {
            moves = moves | forward_and_to_right;
        }
    }
    else {
        if (unoccupied_square(start_pos >> 8, bitboards_ptr)) {
            moves = moves | (start_pos >> 8);
            if (rank(start_pos) == 7 && unoccupied_square(start_pos >> 16, bitboards_ptr)) {
                moves = moves | (start_pos >> 16);
            }
        }
        forward_and_to_left = start_pos >> 9;
        forward_and_to_right = start_pos >> 7;
        if ((opp_bb | ep_target) & (forward_and_to_left & ~FILE_A)) {
            moves = moves | forward_and_to_left;
        }
        if ((opp_bb | ep_target) & (forward_and_to_right & ~FILE_H)) {
            moves = moves | forward_and_to_right;
        }
    }

    return moves;
}

// return true if I'm checked
bool am_i_checked(U64 *bitboards_ptr, bool is_white) {
    U64 single_pos = 1ULL;
    U64 opp_bb = opp_bitboard(is_white, bitboards_ptr);
    U64 king_bb;
    if (is_white) {
        king_bb = bitboards_ptr[WHITE_KING];
    } else {
        king_bb = bitboards_ptr[BLACK_KING];
    }
    // king algebraic notation string
    char king_an[3];
    strcpy(king_an, bitboard_to_an(king_bb));
    for (int i=0; i< 64; i++) {
        if (single_pos & opp_bb) {
            // moves algebraic notation string
            char *moves_an = find_moves(bitboard_to_an(single_pos), bitboards_ptr, false);
            // check if moves algebraic notation string contains king algebraic notation string
            if (strstr(moves_an, king_an) != NULL) {
                return true;
            }
        }
        single_pos = single_pos << 1;
    }
    return false;
}

// Return true if I'm checkmated
bool detect_checkmate(bool is_white) {
    U64 my_bb = my_bitboard(is_white, bitboards);
    U64 single_pos = 1ULL;
    char *moves_ptr;
    char *local_an;
    for (int i=0; i<64; i++) {
        if (my_bb & single_pos) {
            strcpy(local_an, bitboard_to_an(single_pos));
            moves_ptr = find_moves(local_an, bitboards, true);
            if (moves_ptr[0]) {
                return false;
            }
        }
        single_pos = single_pos << 1;
    }
    return true;
}

// Return pawn's position if a pawn needs promotion
char *detect_pawn_promotion() {
    U64 pawn_promotion_bb = ((bitboards[WHITE_PAWN] & RANK_8) | (bitboards[BLACK_PAWN] & RANK_1));
    if (pawn_promotion_bb) {
        return bitboard_to_an(pawn_promotion_bb);
    }
    return NULL;
}

// Promote a pawn and return the fen string
char *promote_pawn(char *pawn_pos, int piece_number) {
    // Convert pawn_pos to bitboard
    U64 pawn_pos_bb = an_to_bitboard(pawn_pos);
    // Remove pawn_pos_bb from both pawn bitboards
    bitboards[WHITE_PAWN] = bitboards[WHITE_PAWN] & ~pawn_pos_bb;
    bitboards[BLACK_PAWN] = bitboards[BLACK_PAWN] & ~pawn_pos_bb;
    // Add the promoted piece to its bitboard
    bitboards[piece_number] = bitboards[piece_number] | pawn_pos_bb;
    // Update the fen string and return it
    update_piece_placement();
    return stringify_fen();
}

/* Convert multiple moves from bitboard representation to algebraic notation, prevent self checks at 
depth zero, and store in moves_ptr */
void update_moves(U64 bitboard, char *start_pos, bool is_white, bool check_for_checks) {
    char *moves_ptr;
    // if we are checking for self checks then we are at depth zero and will use the primary moves array
    if (check_for_checks) {
        moves_ptr = primary_moves_arr;
    /* otherwise we are at depth one and will use the secondary moves array so as not to overwrite the 
    primary moves array */
    } else {
        moves_ptr = secondary_moves_arr;
    }
    int moves_ptr_index = 0;
    U64 single_pos = 1ULL;
    U64 local_bitboards[12];
    Fen local_fen;
    char file;
    char rank;
    for (int i = 0; i < 64; i++) {
        if (bitboard & single_pos) {
            // Determine File
            file = ('h' - (i % 8));
            // Determine Rank
            rank = ((i / 8) + '1');
            char end_pos[] = {file, rank, '\0'};
            // Copy bitboards and fen to local variables
            memcpy(local_bitboards, bitboards, sizeof(bitboards));
            *(local_fen.piece_placement) = *(fen.piece_placement);
            local_fen.active_color = fen.active_color;
            *(local_fen.castling_availability) = *(fen.castling_availability);
            *(local_fen.en_passant_target) = *(fen.en_passant_target);
            local_fen.halfmove_clock = fen.halfmove_clock;
            local_fen.fullmove_number = fen.fullmove_number;
            // Make the move on our local copy of the bitboards
            process_move(start_pos, end_pos, local_bitboards, &local_fen);
            // Prevent self-check
            if (check_for_checks) {
                // Detect if I'm checked on the local bitboards instance
                if (!am_i_checked(local_bitboards, is_white)) {
                    // Assign File
                    moves_ptr[moves_ptr_index] = file;
                    moves_ptr_index++;
                    // Assign Rank
                    moves_ptr[moves_ptr_index] = rank;
                    moves_ptr_index++;
                }
            } else {
                // Assign File
                moves_ptr[moves_ptr_index] = file;
                moves_ptr_index++;
                // Assign Rank
                moves_ptr[moves_ptr_index] = rank;
                moves_ptr_index++;
            }
        }
        single_pos = single_pos << 1;
    }
    // Null terminate the end of the array
    moves_ptr[moves_ptr_index] = '\0';
}

// Read bitboards, determine piece placement, and store it in fen.pieceplacement
void update_piece_placement() {
    // A bitboard with a single occupied square that will be bitshifted down to 1
    U64 square = 1ULL << 63;
    char result[74];
    int result_index = 0;
    // A rolling count of each blank square
    int blank_count = 0;

    // Iterate through 8 rows
    for (int i=0; i<8; i++) {
        blank_count = 0;
        // Iterate through 8 squares in a row
        for (int j=0; j<8; j++) {
            // Iterate through each type of piece
            for (int k=0; k<12; k++) {
                if (bitboards[k] & square) {
                    // check for previous blank squares
                    if (blank_count != 0) {
                        result[result_index] = '0' + blank_count;
                        result_index++;
                        blank_count = 0;
                    }
                    result[result_index] = fen_lookup[k];
                    result_index++;
                    break;
                }
                // Increase blank count if square is unoccupied
                else if (k == 11) {
                    blank_count++;
                }
            }
            // Bitshift down to the next square
            square = square >> 1;
        }
        // Check for blank spaces at the end of the row and add them to the result
        if (blank_count != 0) {
            result[result_index] = '0' + blank_count;
            result_index++;
        }
        // Add slashes to denote a new row
        result[result_index] = '/';
        result_index++;
    }
    // Terminate string at last slash
    result[result_index-1] = 0;
    // Copy result to fen.piece_placement
    strcpy(fen.piece_placement, result);
}

/* Receive the start position in algebraic notation, a pointer to the bitboards representing our 
piece placement, a boolean representing whether to prevent self-checks, and return a pointer to all
legal moves in algebraic notation. */
char *find_moves(char start_pos[], U64* bitboards_ptr, bool check_for_checks) 
{
    U64 moves;
    bool is_white;
    char *moves_ptr;

    if (!bitboards_ptr) {
        bitboards_ptr = bitboards;
    }

    /* If we are checking for self checks, then we are at depth zero and will use 
    the primary moves pointer */
    if (check_for_checks) {
        moves_ptr = primary_moves_arr;
    /* Otherwise we are at depth one and need to use the secondary moves pointer so
    as not to overwrite the primary */
    } else {
        moves_ptr = secondary_moves_arr;
    }
    // Empty contents of moves_ptr
    memset(moves_ptr, '\0', 43);

    U64 start_pos_bb = an_to_bitboard(start_pos);
    for (int i = 0; i < 12; i++) {
        if (start_pos_bb & bitboards_ptr[i]) {
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
                    moves = king_pattern(start_pos_bb, is_white, bitboards_ptr);
                    update_moves(moves, start_pos, is_white, check_for_checks);
                    break;
                // Queen
                case 1:
                    moves = queen_pattern(start_pos_bb, is_white, bitboards_ptr);
                    update_moves(moves, start_pos, is_white, check_for_checks);
                    break;
                // Rook
                case 2:
                    moves = rook_pattern(start_pos_bb, is_white, bitboards_ptr);
                    update_moves(moves, start_pos, is_white, check_for_checks);
                    break;
                // Bishop
                case 3:
                    moves = bishop_pattern(start_pos_bb, is_white, bitboards_ptr);
                    update_moves(moves, start_pos, is_white, check_for_checks);
                    break;
                // Knight
                case 4:
                    moves = knight_pattern(start_pos_bb, is_white, bitboards_ptr);
                    update_moves(moves, start_pos, is_white, check_for_checks);
                    break;
                // Pawn
                case 5:
                    moves = pawn_pattern(start_pos_bb, is_white, bitboards_ptr);
                    update_moves(moves, start_pos, is_white, check_for_checks);
                    break;
                // Default
                default:
                    memset(moves_ptr, '\0', 43);
            }
        }
    }
    return moves_ptr;
}

// Take start and end position of move and use it to update bitboards_arr, all of fen_ptr except piece_placement
void process_move(char start_pos[], char end_pos[], U64 bitboards_arr[], Fen* fen_ptr) {
    bool is_white;
    int piece_type;

    U64 start_pos_bb = an_to_bitboard(start_pos);
    U64 end_pos_bb = an_to_bitboard(end_pos);
    for (int i = 0; i < 12; i++) {
        if (start_pos_bb & bitboards_arr[i]) {
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
                bitboards_arr[i] = bitboards_arr[i] & ~start_pos_bb;
                // And the bitwise complement of our end position to each bitboard to remove any captured piece
                for (int j = 0; j < 12; j++) {
                    bitboards_arr[j] = bitboards_arr[j] & ~end_pos_bb;
                }
                // Or the end position to our bitboard to add it to the end position
                bitboards_arr[i] = bitboards_arr[i] | end_pos_bb;
                // Update En Passant target
                strcpy((*fen_ptr).en_passant_target, "-");
                // Update castling availability
                if (piece_type == 2) {
                    if (start_pos_bb == 1ULL) {
                        remove_chars((*fen_ptr).castling_availability, 'K');
                    }
                    else if (start_pos_bb == 128ULL) {
                        remove_chars((*fen_ptr).castling_availability, 'Q');
                    }
                    else if (start_pos_bb == 72057594037927936ULL) {
                        remove_chars((*fen_ptr).castling_availability, 'k');
                    }
                    else if (start_pos_bb == 9223372036854775808ULL) {
                        remove_chars((*fen_ptr).castling_availability, 'q');
                    }
                }
            }
            // King
            else if (piece_type == 0) {
                // Castling
                // King side castling
                if (start_pos_bb >> 2 == end_pos_bb) {
                    // Remove rook
                    bitboards_arr[i+2] = bitboards_arr[i+2] & ~(end_pos_bb >> 1);
                    // Add rook
                    bitboards_arr[i+2] = bitboards_arr[i+2] | (end_pos_bb << 1);
                }
                // Queen side castling
                else if (start_pos_bb << 2 == end_pos_bb) {
                    // Remove rook
                    bitboards_arr[i+2] = bitboards_arr[i+2] & ~(end_pos_bb << 2);
                    // Add rook
                    bitboards_arr[i+2] = bitboards_arr[i+2] | (end_pos_bb >> 1);
                }
                // And the bitwise complement of our start position to our bitboard to remove it from start position
                bitboards_arr[i] = bitboards_arr[i] & ~start_pos_bb;
                // And the bitwise complement of our end position to each bitboard to remove any captured piece
                for (int j = 0; j < 12; j++) {
                    bitboards_arr[j] = bitboards_arr[j] & ~end_pos_bb;
                }
                // Or the end position to our bitboard to add it to the end position
                bitboards_arr[i] = bitboards_arr[i] | end_pos_bb;
                // Update castling availability
                if (is_white) {
                    remove_chars((*fen_ptr).castling_availability, 'K');
                    remove_chars((*fen_ptr).castling_availability, 'Q');
                }
                else {
                    remove_chars((*fen_ptr).castling_availability, 'k');
                    remove_chars((*fen_ptr).castling_availability, 'q');
                }
                // Update En Passant target
                strcpy((*fen_ptr).en_passant_target, "-");
            }
            // Pawn
            else if (piece_type == 5) {
                // Non-capture
                if (file(start_pos_bb) == file(end_pos_bb)) {
                    // And the bitwise complement of our start position to our bitboard to remove it from start position
                    bitboards_arr[i] = bitboards_arr[i] & ~start_pos_bb;
                    // Or the end position to our bitboard to add it to the end position
                    bitboards_arr[i] = bitboards_arr[i] | end_pos_bb;
                    // Pawn double move
                    // White
                    if ((start_pos_bb << 16) == end_pos_bb) {
                        // Update En Passant target
                        strcpy((*fen_ptr).en_passant_target, bitboard_to_an(start_pos_bb << 8));
                    } 
                    // Black
                    else if ((start_pos_bb >> 16) == end_pos_bb) {
                        // Update En Passant target
                        strcpy((*fen_ptr).en_passant_target, bitboard_to_an(start_pos_bb >> 8));
                    }
                    else {
                        // Update En Passant target
                        strcpy((*fen_ptr).en_passant_target, "-");
                    }
                }
                // En Passant
                else if (strcmp(end_pos, fen.en_passant_target) == 0) {
                    if (is_white) {
                        // Remove captured pawn
                        bitboards_arr[11] = bitboards_arr[11] & ~(end_pos_bb >> 8);
                    }
                    else {
                        // Remove captured pawn
                        bitboards_arr[5] = bitboards_arr[5] & ~(end_pos_bb << 8);
                    }
                    // And the bitwise complement of our start position to our bitboard to remove it from start position
                    bitboards_arr[i] = bitboards_arr[i] & ~start_pos_bb;
                    // Or the end position to our bitboard to add it to the end position
                    bitboards_arr[i] = bitboards_arr[i] | end_pos_bb;
                }
                // Regular capture
                else {
                    // And the bitwise complement of our start position to our bitboard to remove it from start position
                    bitboards_arr[i] = bitboards_arr[i] & ~start_pos_bb;
                    // And the bitwise complement of our end position to each bitboard to remove any captured piece
                    for (int j = 0; j < 12; j++) {
                        bitboards_arr[j] = bitboards_arr[j] & ~end_pos_bb;
                    }
                    // Or the end position to our bitboard to add it to the end position
                    bitboards_arr[i] = bitboards_arr[i] | end_pos_bb;
                }
            }
        }
    }
    // Update active color
    if (is_white) {
        (*fen_ptr).active_color = 'b';
    } else {
        (*fen_ptr).active_color = 'w';
    }
}

char *make_move(char start_pos[], char end_pos[]) 
{
    process_move(start_pos, end_pos, bitboards, &fen);
    update_piece_placement();
    return stringify_fen();
}