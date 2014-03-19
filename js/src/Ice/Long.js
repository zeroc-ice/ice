// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");

    //
    // The Long type represents a signed 64-bit integer as two 32-bit values
    // corresponding to the high and low words.
    //
    var Ice = global.Ice || {};

    var Long = Ice.Class({
        __init__: function(high, low)
        {
            this.high = high;
            this.low = low;
        },
        hashCode: function()
        {
            return this.low;
        },
        equals: function(rhs)
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
        },
        toString: function()
        {
            return this.high + ":" + this.low;
        },
        toNumber: function()
        {
            if((this.high & Long.SIGN_MASK) != 0)
            {
                var low = ~this.low;
                var high = ~this.high;
                if(low < 0xFFFFFFFF)
                {
                    low += 1;
                }
                else
                {
                    low = 0;
                    high += 1;
                    if(high > Long.HIGH_MAX)
                    {
                        return Number.NEGATIVE_INFINITY;
                    }
                }
                return -1 * (high * Long.HIGH_MASK) + low;
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
    });

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
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
