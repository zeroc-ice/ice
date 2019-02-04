//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

//
// The Long type represents a signed 64-bit integer as two 32-bit values
// corresponding to the high and low words.
//
class Long
{
    //
    // If only one argument is provide we assume it is a JavaScript Number,
    // and we convert it to two 32 bit words to fit in the Ice.Long internal
    // representation.
    //
    // If two arguments are provided we asume these are the high and low words
    // respectively.
    //
    constructor(high = 0, low = undefined)
    {
        if(!Number.isSafeInteger(high))
        {
            throw new RangeError(low === undefined ?
                    "Number must be a safe integer" :
                    "High word must be a safe integer");
        }

        if(low === undefined)
        {
            this.low = high >>> 0;
            this.high = ((high - this.low) / Long.HIGH_MASK) >>> 0;
        }
        else
        {
            if(!Number.isSafeInteger(low))
            {
                throw new RangeError("Low word must be a safe integer");
            }
            if(low < 0 || low > Long.MAX_UINT32)
            {
                throw new RangeError("Low word must be between 0 and 0xFFFFFFFF");
            }
            if(high < 0 || high > Long.MAX_UINT32)
            {
                throw new RangeError("High word must be between 0 and 0xFFFFFFFF");
            }

            this.high = high;
            this.low = low;
        }
    }

    hashCode()
    {
        return this.low;
    }

    equals(rhs)
    {
        if(this === rhs)
        {
            return true;
        }
        if(!(rhs instanceof Long))
        {
            return false;
        }
        return this.high === rhs.high && this.low === rhs.low;
    }

    toString()
    {
        return this.high + ":" + this.low;
    }

    toNumber()
    {
        if((this.high & Long.SIGN_MASK) !== 0)
        {
            const l = (~this.low) >>> 0;
            const h = (~this.high) >>> 0;
            if(h > Long.HIGH_MAX || h == Long.HIGH_MAX && l == Long.MAX_UINT32)
            {
                return Number.NEGATIVE_INFINITY;
            }
            return -((h * Long.HIGH_MASK) + l + 1);
        }
        else
        {
            if(this.high > Long.HIGH_MAX)
            {
                return Number.POSITIVE_INFINITY;
            }
            return (this.high * Long.HIGH_MASK) + this.low;
        }
    }
}

//
// 2^32
//
Long.MAX_UINT32 = 0xFFFFFFFF;

//
// (high & SIGN_MASK) != 0 denotes a negative number;
// that is, the most significant bit is set.
//
Long.SIGN_MASK = 0x80000000;

//
// When converting to a JavaScript Number we left shift the
// high word by 32 bits. As that isn't possible using JavaScript's
// left shift operator, we multiply the value by 2^32 which will
// produce the same result.
//
Long.HIGH_MASK = 0x100000000;

//
// The maximum value for the high word when coverting to
// a JavaScript Number is 2^21 - 1, in which case all
// 53 bits are used.
//
Long.HIGH_MAX = 0x1FFFFF;

Ice.Long = Long;
module.exports.Ice = Ice;
