import { u32, Bitboard } from "/u64.js";

const pieceLetters = {
    white: ['P', 'N', 'B', 'R', 'Q', 'K'],
    black: ['p', 'n', 'b', 'r', 'q', 'k']
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