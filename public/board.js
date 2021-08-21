import { u32, Bitboard } from "/u64.js";

class Board {
    constructor() {
        this.bitboards = {};
        this.initDefaultBitboards();
    }

    move() {
        return;
    }

    findMoves() {
        return;
    }

    initDefaultBitboards() {
        this.bitboards['P'] = new Bitboard(0x00000000, 0x0000ff00);
        this.bitboards['N'] = new Bitboard(0x00000000, 0x00000042);
        this.bitboards['B'] = new Bitboard(0x00000000, 0x00000024);
        this.bitboards['R'] = new Bitboard(0x00000000, 0x00000081);
        this.bitboards['Q'] = new Bitboard(0x00000000, 0x00000010);
        this.bitboards['K'] = new Bitboard(0x00000000, 0x00000008);
        this.bitboards['p'] = new Bitboard(0x00ff0000, 0x00000000);
        this.bitboards['n'] = new Bitboard(0x42000000, 0x00000000);
        this.bitboards['b'] = new Bitboard(0x24000000, 0x00000000);
        this.bitboards['r'] = new Bitboard(0x81000000, 0x00000000);
        this.bitboards['q'] = new Bitboard(0x10000000, 0x00000000);
        this.bitboards['k'] = new Bitboard(0x08000000, 0x00000000);
    }
}