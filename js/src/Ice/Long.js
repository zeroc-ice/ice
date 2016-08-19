// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;

//
// The Long type represents a signed 64-bit integer as two 32-bit values
// corresponding to the high and low words.
//
class Long
{
    constructor(high, low)
    {
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
            if(this.high === Long.MAX_UINT32 && this.low !== 0)
            {
                return -(~this.low + 1);
            }
 
            const high = ~this.high + 1;

            if(high > Long.HIGH_MAX)
            {
                return Number.NEGATIVE_INFINITY;
            }

            return -1 * (high * Long.HIGH_MASK) + this.low;
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
