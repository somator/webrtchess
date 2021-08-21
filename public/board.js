import { u32, Bitboard } from "/u64.js";

class Board {
    constructor() {
        this.bitboards = {};
        this.initDefaultBitboards();
    }

    move() {
        return;
    }

    findMoves(piece, pos) {
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
}