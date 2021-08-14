// return unsigned 32-bit Integer
function u32(n) {
    return n >>> 0;
}

export default class Bitboard {
    constructor(upper, lower) {
        this.upper = u32(upper);
        this.lower = u32(lower);
    }

    NOT() {
        return new Bitboard(u32(~this.upper), u32(~this.lower));
    }

    AND(other) {
        return new Bitboard(u32(this.upper & other.upper), u32(this.lower & other.lower));
    }

    OR(other) {
        return new Bitboard(u32(this.upper | other.upper), u32(this.lower | other.lower));
    }

    XOR(other) {
        return new Bitboard(u32(this.upper ^ other.upper), u32(this.upper ^ other.upper));
    }

    equals(other) {
        return (this.lower == other.lower && this.upper == other.upper)
    }

    SHL(n) {
        n = u32(n);
        if (n > 31) {
            return new Bitboard(u32(this.upper << (n - 32)), u32(0));
        } else {
            return new Bitboard(u32((this.upper << n) | (this.lower >>> (32 - n))), u32(this.lower << n));
        }
    }

    SHR(n) {
        n = u32(n);
        if (n > 31) {
            return new Bitboard(u32(0), (this.upper >>> (n - 32)));
        } else {
            return new Bitboard((this.upper >>> n), u32((this.lower >>> n) | (this.upper << (32 - n))));
        }
    }
}