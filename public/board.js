import { u32, Bitboard } from "/u64.js";

const pieceLetters = {
    white: ['P', 'N', 'B', 'R', 'Q', 'K'],
    black: ['p', 'n', 'b', 'r', 'q', 'k'],
};

const files = {
    'a' : new Bitboard(0x80808080, 0x80808080),
    'b' : new Bitboard(0x40404040, 0x40404040),
    'c' : new Bitboard(0x20202020, 0x20202020),
    'd' : new Bitboard(0x10101010, 0x10101010),
    'e' : new Bitboard(0x08080808, 0x08080808),
    'f' : new Bitboard(0x04040404, 0x04040404),
    'g' : new Bitboard(0x02020202, 0x02020202),
    'h' : new Bitboard(0x01010101, 0x01010101),
};

const ranks = {
    '1' : new Bitboard(0x00000000, 0x000000ff),
    '2' : new Bitboard(0x00000000, 0x0000ff00),
    '3' : new Bitboard(0x00000000, 0x00ff0000),
    '4' : new Bitboard(0x00000000, 0xff000000),
    '5' : new Bitboard(0x000000ff, 0x00000000),
    '6' : new Bitboard(0x0000ff00, 0x00000000),
    '7' : new Bitboard(0x00ff0000, 0x00000000),
    '8' : new Bitboard(0xff000000, 0x00000000),
};

class Board {
    constructor() {
        this.bitboards = {};
        this.initDefaultBitboards();
    }

    move() {
        return;
    }

    findMoves(pos, piece) {
        switch (piece.type) {
            case 'pawn':
                moves = pawnPattern()
                break;
            case 'knight':
                moves = knightPattern();
                break;
            case 'bishop':
                moves = bishopPattern();
                break;
            case 'rook':
                moves = rookPattern();
                break;
            case 'queen':
                moves = queenPattern();
                break;
            case 'king':
                moves = kingPattern();
                break;
        }
        return moves;
    }

    pawnPattern() {
        return;
    }

    knightPattern() {
        return;
    }

    bishopPattern() {
        return;
    }

    rookPattern() {
        return;
    }

    queenPattern() {
        return;
    }

    kingPattern() {
        return;
    }

    get whiteBitBoard() {
        whiteBitboard = new Bitboard(0, 0);
        for (pieceLetter of pieceLetters.white) {
            whiteBitboard = whiteBitboard.OR(bitboards[pieceLetter]);
        }
        return whiteBitboard;
    }

    get blackBitboard() {
        blackBitboard = new Bitboard(0, 0);
        for (pieceLetter of pieceLetters.black) {
            blackBitboard = blackBitboard.OR(bitboards[pieceLetter]);
        }
        return blackBitboard;
    }

    get allBitboard() {
        return this.whiteBitBoard.OR(this.blackBitboard);
    }
}