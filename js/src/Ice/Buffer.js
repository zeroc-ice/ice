// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// Ice.Buffer implementation to be used by Node.js, it uses node Buffer
// as the store.
//

//
// Define Node.Buffer as an alias to NodeJS global Buffer type,
// that allow us to refer to Ice.Buffer as Buffer in this file.
//
var Node = { Buffer: global.Buffer };

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Long", "../Ice/Class"]);

var Long = Ice.Long;

var __BufferOverflowException__ = "BufferOverflowException";
var __BufferUnderflowException__ = "BufferUnderflowException";
var __IndexOutOfBoundsException__ = "IndexOutOfBoundsException";

var Buffer = Ice.Class({
    __init__: function(buffer)
    {
        if(buffer !== undefined)
        {
            this.b = buffer;
        }
        else
        {
            this.b = null;
        }
        this._position = 0;
        this._limit = 0;
        this._shrinkCounter = 0;
    },
    empty: function()
    {
        return this._limit === 0;
    },
    resize: function(n)
    {
        if(n === 0)
        {
            this.clear();
        }
        else if(n > this.capacity)
        {
            this.reserve(n);
        }
        this._limit = n;
    },
    clear: function()
    {
        this.b = null;
        this._position = 0;
        this._limit = 0;
    },
    //
    // Call expand(n) to add room for n additional bytes. Note that expand()
    // examines the current position of the buffer first; we don't want to
    // expand the buffer if the caller is writing to a location that is
    // already in the buffer.
    //
    expand: function(n)
    {
        var sz = this.capacity === 0 ? n : this._position + n;
        if(sz > this._limit)
        {
            this.resize(sz);
        }
    },
    reset: function()
    {
        if(this._limit > 0 && this._limit * 2 < this.capacity)
        {
            //
            // If the current buffer size is smaller than the
            // buffer capacity, we shrink the buffer memory to the
            // current size. This is to avoid holding on to too much
            // memory if it's not needed anymore.
            //
            if(++this._shrinkCounter > 2)
            {
                this.reserve(this._limit);
                this._shrinkCounter = 0;
            }
        }
        else
        {
            this._shrinkCounter = 0;
        }
        this._limit = this.capacity();
        this._position = 0;
    },
    reserve: function(n)
    {
        var b, capacity;
        if(n > this.capacity)
        {
            capacity = Math.max(n, 2 * this.capacity);
            capacity = Math.max(1024, capacity);
            if(this.b === null)
            {
                this.b = new Node.Buffer(capacity);
            }
            else
            {
                b = new Node.Buffer(capacity);
                this.b.copy(b);
                this.b = b;
            }
        }
        else if(n < this.capacity)
        {
            this.b = this.b.slice(0, n);
        }
    },
    put: function(v)
    {
        if(this._position === this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.b.writeUInt8(v, this._position, true);
        this._position++;
    },
    putAt: function(i, v)
    {
        if(i >= this._limit)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        this.b.writeUInt8(v, i, true);
    },
    putArray: function(v)
    {
        //Expects a Nodejs Buffer
        if(!Node.Buffer.isBuffer(v))
        {
            throw new TypeError("argument is not a Node.Buffer");
        }
        if(this._position + v.length > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        v.copy(this.b, this._position);
        this._position += v.length;
    },
    putShort: function(v)
    {
        if(this._position + 2 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.b.writeInt16LE(v, this._position, true);
        this._position += 2;
    },
    putShortAt: function(i, v)
    {
        if(i + 2 > this._limit || i < 0)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        this.b.writeInt16LE(v, i, true);
    },
    putInt: function(v)
    {
        if(this._position + 4 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.b.writeInt32LE(v, this._position, true);
        this._position += 4;
    },
    putIntAt: function(i, v)
    {
        if(i + 4 > this._limit || i < 0)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        this.b.writeInt32LE(v, i, true);
    },
    putFloat: function(v)
    {
        if(this._position + 4 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.b.writeFloatLE(v, this._position, true);
        this._position += 4;
    },
    putFloatAt: function(i, v)
    {
        if(i + 4 > this._limit || i < 0)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        this.b.writeFloatLE(v, i, true);
    },
    putDouble: function(v)
    {
        if(this._position + 8 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.b.writeDoubleLE(v, this._position, true);
        this._position += 8;
    },
    putDoubleAt: function(i, v)
    {
        if(i + 8 > this._limit || i < 0)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        this.b.writeDoubleLE(v, i, true);
    },
    putLong: function(v)
    {
        if(this._position + 8 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.b.writeUInt32LE(v.low, this._position, true);
        this._position += 4;
        this.b.writeUInt32LE(v.high, this._position, true);
        this._position += 4;
    },
    putLongAt: function(i, v)
    {
        if(i + 8 > this._limit || i < 0)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        this.b.writeUInt32LE(v.low, i, true);
        this.b.writeUInt32LE(v.high, i + 4, true);
    },
    writeString: function(stream, v)
    {
        var sz = Node.Buffer.byteLength(v);
        stream.writeSize(sz);
        stream.expand(sz);
        this.putString(v, sz);
    },
    putString: function(v, sz)
    {
        if(this._position + sz > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        var bytes = this.b.write(v, this._position);
        //
        // Check all bytes were written
        //
        if(bytes < sz)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        this._position += sz;
    },
    get: function()
    {
        if(this._position >= this._limit)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.b.readUInt8(this._position, true);
        this._position++;
        return v;
    },
    getAt: function(i)
    {
        if(i < 0 || i >= this._limit)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        return this.b.readUInt8(i, true);
    },
    getArray: function(length)
    {
        if(this._position + length > this._limit)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var buffer = new Node.Buffer(length);
        this.b.slice(this._position, this._position + length).copy(buffer);
        this._position += length;
        return buffer;
    },
    getArrayAt: function(position, length)
    {
        if(position + length > this._limit)
        {
            throw new Error(__BufferUnderflowException__);
        }
        length = length === undefined ? (this.b.length - position) : length;
        var buffer = new Node.Buffer(length);
        this.b.slice(position, position + length).copy(buffer);
        return buffer;
    },
    getShort: function()
    {
        var v;
        if(this._limit - this._position < 2)
        {
            throw new Error(__BufferUnderflowException__);
        }
        v = this.b.readInt16LE(this._position, true);
        this._position += 2;
        return v;
    },
    getInt: function()
    {
        var v;
        if(this._limit - this._position < 4)
        {
            throw new Error(__BufferUnderflowException__);
        }
        v = this.b.readInt32LE(this._position, true);
        this._position += 4;
        return v;
    },
    getFloat: function()
    {
        if(this._limit - this._position < 4)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.b.readFloatLE(this._position, true);
        this._position += 4;
        return v;
    },
    getDouble: function()
    {
        if(this._limit - this._position < 8)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.b.readDoubleLE(this._position, true);
        this._position += 8;
        return v;
    },
    getLong: function()
    {
        if(this._limit - this._position < 8)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = new Long();
        v.low = this.b.readUInt32LE(this._position, true);
        this._position += 4;
        v.high = this.b.readUInt32LE(this._position, true);
        this._position += 4;
        return v;
    },
    getString: function(length)
    {
        if(this._position + length > this._limit)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var s =this.b.toString("utf8", this._position, this._position + length);
        this._position += length;
        return s;
    }
});

var prototype = Buffer.prototype;

Object.defineProperty(prototype, "position", {
    get: function() { return this._position; },
    set: function(position){
        if(position >= 0 && position <= this._limit)
        {
            this._position = position;
        }
    }
});

Object.defineProperty(prototype, "limit", {
    get: function() { return this._limit; },
    set: function(limit){
        if(limit <= this.capacity)
        {
            this._limit = limit;
            if(this._position > limit)
            {
                this._position = limit;
            }
        }
    }
});

Object.defineProperty(prototype, "capacity", {
    get: function() { return this.b === null ? 0 : this.b.length; }
});

Object.defineProperty(prototype, "remaining", {
    get: function() { return this._limit - this._position; }
});

//
// Create a native buffer from an array of bytes.
//
Buffer.createNative = function(data)
{
    if(data === undefined)
    {
        return new Node.Buffer(0);
    }
    else
    {
        return new Node.Buffer(data);
    }
};

Ice.Buffer = Buffer;
module.exports.Ice = Ice;
