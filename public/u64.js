// return unsigned 32-bit Integer
function u32(n) {
    return n >>> 0;
}

class Bitboard {
    constructor(upper, lower) {
        this.upper = u32(upper);
        this.lower = u32(lower);
    }
}