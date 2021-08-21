import { u32, Bitboard } from "/u64.js";

const emptyBb = new Bitboard(0,0);

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

function notFiles(arr) {
    bitboard = new Bitboard(0,0);
    for (file in arr) {
        bitboard = bitboard.AND(files[file]);
    }
    return bitboard.NOT();
}

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
                moves = pawnPattern(pos, piece.color);
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

    pawnPattern(pos, color) {
        let moves = new Bitboard(0, 0);
        const notAllBb = this.allBitboard.NOT();
        if (color == 'white') {
            const oppBb = this.blackBitboard;
            moves = moves.OR(pos.LSH(8).AND(notAllBb));
            moves = moves.OR(pos.LSH(16).AND(ranks['4']).AND(notAllBb));
            moves = moves.OR(pos.LSH(9).AND(files['h'].NOT()).AND(oppBb));
            moves = moves.OR(pos.LSH(7).AND(files['a'].NOT()).AND(oppBb))
        }
        else if (color == 'black') {
            const oppBb = this.whiteBitBoard;
            moves = moves.OR(pos.RSH(8).AND(notAllBb));
            moves = moves.OR(pos.RSH(16).AND(ranks['5']).AND(notAllBb));
            moves = moves.OR(pos.RSH(9).AND(files['a'].NOT()).AND(oppBb));
            moves = moves.OR(pos.RSH(7).AND(files['h'].NOT()).AND(oppBb))
        }
        return moves;
    }

    knightPattern(pos, color) {
        let moves = new Bitboard(0, 0);
        const notMyBb = this.bitboardOfColor(color).NOT();
        moves = moves.OR(pos.LSH(17).AND(notFiles(['a'])));
        moves = moves.OR(pos.LSH(10).AND(notFiles(['a', 'b'])));
        moves = moves.OR(pos.RSH( 6).AND(notFiles(['a', 'b'])));
        moves = moves.OR(pos.RSH(15).AND(notFiles(['a'])));
        moves = moves.OR(pos.LSH(15).AND(notFiles(['h'])));
        moves = moves.OR(pos.LSH( 6).AND(notFiles(['g', 'h'])));
        moves = moves.OR(pos.RSH(10).AND(notFiles(['g', 'h'])));
        moves = moves.OR(pos.RSH(17).AND(notFiles(['h'])));
        moves = moves.AND(notMyBb);
        return moves;
    }

    bishopPattern(pos, color) {
        let moves = new Bitboard(0, 0);
        if (color == 'white') {
            const notMyBb = this.whiteBitBoard.NOT();
            const oppBb = this.blackBitboard;
        } else {
            const notMyBb = this.blackBitBoard.NOT();
            const oppBb = this.whiteBitboard;
        }
        let attacking = emptyBb;
        let NW = pos;
        while (!NW.isEmpty() && attacking.isEmpty()) {
            NW = NW.LSH(9).AND(files['h'].NOT().AND(ranks['1'].NOT())).AND(notMyBb);
            attacking = NW.AND(oppBb);
            moves = moves.OR(NW);
        }
        attacking = emptyBb;
        let NE = pos;
        while (!NE.isEmpty() && attacking.isEmpty()) {
            NE = NE.LSH(7).AND(files['a'].NOT().AND(ranks['1'].NOT())).AND(notMyBb);
            attacking = NE.AND(oppBb);
            moves = moves.OR(NE);
        }
        attacking = emptyBb;
        let SE = pos;
        while (!SE.isEmpty() && attacking.isEmpty()) {
            SE = SE.RSH(9).AND(files['a'].NOT().AND(ranks['8'].NOT())).AND(notMyBb);
            attacking = SE.AND(oppBb);
            moves = moves.OR(SE);
        }
        attacking = emptyBb;
        let SW = pos;
        while (!SW.isEmpty() && attacking.isEmpty()) {
            SW = SW.RSH(7).AND(files['h'].NOT().AND(ranks['8'].NOT())).AND(notMyBb);
            attacking = SW.AND(oppBb);
            moves = moves.OR(SW);
        }
        return moves;
    }

    rookPattern(pos, color) {
        let moves = new Bitboard(0, 0);
        let attacking = emptyBb;
        if (color == 'white') {
            const notMyBb = this.whiteBitBoard.NOT();
            const oppBb = this.blackBitboard;
        } else {
            const notMyBb = this.blackBitBoard.NOT();
            const oppBb = this.whiteBitboard;
        }
        let N = pos;
        while (!N.isEmpty() && attacking.isEmpty()) {
            N = N.LSH(8).AND(ranks['1'].NOT()).AND(notMyBb);
            attacking = N.AND(oppBb);
            moves = moves.OR(N);
        }
        attacking = emptyBb;
        let E = pos;
        while (!E.isEmpty() && attacking.isEmpty()) {
            E = E.RSH(1).AND(files['a'].NOT()).AND(notMyBb);
            attacking = E.AND(oppBb);
            moves = moves.OR(E);
        }
        attacking = emptyBb;
        let S = pos;
        while (!S.isEmpty() && attacking.isEmpty()) {
            S = S.RSH(8).and(ranks['8'].NOT()).AND(notMyBb);
            attacking = S.AND(oppBb);
            moves = moves.OR(S)
        }
        attacking = emptyBb;
        let W = pos;
        while (!W.isEmpty() && attacking.isEmpty()) {
            W = W.LSH(1).AND(files['h'].NOT()).AND(notMyBb);
            attacking = W.AND(oppBb);
            moves = moves.OR(W);
        }
        return moves;
    }

    queenPattern(pos, color) {
        moves = this.bishopPattern(pos, color);
        moves = moves.OR(this.rookPattern(pos, color));
        return moves;
    }

    kingPattern() {
        return;
    }

    bitboardOfColor(color) {
        bitboard = new Bitboard(0, 0);
        for (pieceLetter of pieceLetters[color]) {
            bitboard = bitboard.OR(this.bitboards[pieceLetter]);
        }
        return bitboard;
    }

    get whiteBitBoard() {
        return this.bitboardOfColor('white');
    }

    get blackBitboard() {
        return this.bitboardOfColor('black');
    }

    get allBitboard() {
        return this.whiteBitBoard.OR(this.blackBitboard);
    }
}