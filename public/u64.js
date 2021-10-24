// return unsigned 32-bit Integer
export function u32(n) {
    return n >>> 0;
}

export class Bitboard {
    /*constructor(upper, lower) {
        this.upper = u32(upper);
        this.lower = u32(lower);
    }*/
    constructor() {
        if (arguments.length == 1) {
            this.upper = u32(parseInt(arguments[0].slice(0, 32)), 2);
            this.lower = u32(parseInt(arguments[0].slice(32, 64)), 2);
        } else {
            this.upper = u32(arguments[0]);
            this.lower = u32(arguments[1]);
        }
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
        console.log(this.upper);
        console.log(other.upper);
        console.log(this.lower);
        console.log(other.upper);
        return (this.lower == other.lower && this.upper == other.upper)
    }

    LSH(n) {
        n = u32(n);
        if (n > 31) {
            return new Bitboard(u32(this.upper << (n - 32)), u32(0));
        } else {
            return new Bitboard(u32((this.upper << n) | (this.lower >>> (32 - n))), u32(this.lower << n));
        }
    }

    RSH(n) {
        n = u32(n);
        if (n > 31) {
            return new Bitboard(u32(0), (this.upper >>> (n - 32)));
        } else {
            return new Bitboard((this.upper >>> n), u32((this.lower >>> n) | (this.upper << (32 - n))));
        }
    }

    isEmpty() {
        return (this.lower == 0 && this.upper == 0)
    }

    getBit(i) {
        i = u32(i);
        return (i < 32 ? ((this.upper & (1 << (31 - i))) >>> (31 - i)) : ((this.lower & (1 << (63 - i))) >>> (63 - i)));
    }

    print() {
        let index = 0;
        let representation = '';
        for (let i = 0; i < 8; i++) {
            let row = '';
            for (let j = 0; j < 8; j++) {
                row = row + this.getBit(index).toString();
                index += 1;
            }
            representation = representation + row + '\n';
        }
        return representation;
    }
}