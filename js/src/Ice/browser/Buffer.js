// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;

Ice.__M.require(module, ["../Ice/Class", "../Ice/Long"]);

var Long = Ice.Long;

//
// IE 10 doesn't implement ArrayBuffer.slice
//

if(!ArrayBuffer.prototype.slice)
{
    ArrayBuffer.prototype.slice = function (start, end)
    {
        var b = new Uint8Array(this);
        end = end === undefined ? b.length : end;
        var result = new Uint8Array(new ArrayBuffer(end - start));
        for(var i = 0, length = result.length; i < length; i++)
        {
            result[i] = b[i + start];
        }
        return result.buffer;
    };
}

var __BufferOverflowException__ = "BufferOverflowException";
var __BufferUnderflowException__ = "BufferUnderflowException";
var __IndexOutOfBoundsException__ = "IndexOutOfBoundsException";

//
// Buffer implementation to be used by web browsers, it uses ArrayBuffer as
// the store.
//
var Buffer = Ice.Class({
    __init__: function(buffer)
    {
        if(buffer !== undefined)
        {
            this.b = buffer;
            this.v = new DataView(this.b);
        }
        else
        {
            this.b = null; // ArrayBuffer
            this.v = null; // DataView
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
        this.v = null;
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
        this._limit = 0;
        this._position = 0;
    },
    reserve: function(n)
    {
        if(n > this.capacity)
        {
            var capacity = Math.max(n, 2 * this.capacity);
            capacity = Math.max(1024, capacity);
            if(!this.b)
            {
                this.b = new ArrayBuffer(capacity);
            }
            else
            {
                var b = new Uint8Array(capacity);
                b.set(new Uint8Array(this.b));
                this.b = b.buffer;
            }
            this.v = new DataView(this.b);
        }
        else if(n < this.capacity)
        {
            this.b = this.b.slice(0, this.capacity);
            this.v = new DataView(this.b);
        }
        else
        {
            return;
        }
    },
    put: function(v)
    {
        if(this._position === this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setUint8(this._position, v);
        this._position++;
    },
    putAt: function(i, v)
    {
        if(i >= this._limit)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        this.v.setUint8(i, v);
    },
    putArray: function(v)
    {
        //Expects an Uint8Array
        if(!(v instanceof Uint8Array))
        {
            throw new TypeError('argument is not a Uint8Array');
        }
        if(v.byteLength > 0)
        {
            if(this._position + v.length > this._limit)
            {
                throw new Error(__BufferOverflowException__);
            }
            new Uint8Array(this.b, 0, this.b.byteLength).set(v, this._position);
            this._position += v.byteLength;
        }
    },
    putShort: function(v)
    {
        if(this._position + 2 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setInt16(this._position, v, true);
        this._position += 2;
    },
    putInt: function(v)
    {
        if(this._position + 4 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setInt32(this._position, v, true);
        this._position += 4;
    },
    putIntAt: function(i, v)
    {
        if(i + 4 > this._limit || i < 0)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        this.v.setInt32(i, v, true);
    },
    putFloat: function(v)
    {
        if(this._position + 4 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setFloat32(this._position, v, true);
        this._position += 4;
    },
    putDouble: function(v)
    {
        if(this._position + 8 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setFloat64(this._position, v, true);
        this._position += 8;
    },
    putLong: function(v)
    {
        if(this._position + 8 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setInt32(this._position, v.low, true);
        this._position += 4;
        this.v.setInt32(this._position, v.high, true);
        this._position += 4;
    },
    writeString: function(stream, v)
    {
        //
        // Encode the string as utf8
        //
        var encoded = unescape(encodeURIComponent(v));

        stream.writeSize(encoded.length);
        stream.expand(encoded.length);
        this.putString(encoded, encoded.length);
    },
    putString: function(v, sz)
    {
        if(this._position + sz > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        for(var i = 0; i < sz; ++i)
        {
            this.v.setUint8(this._position, v.charCodeAt(i));
            this._position++;
        }
    },
    get: function()
    {
        if(this._position >= this._limit)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.v.getUint8(this._position);
        this._position++;
        return v;
    },
    getAt: function(i)
    {
        if(i < 0 || i >= this._limit)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        return this.v.getUint8(i);
    },
    getArray: function(length)
    {
        if(this._position + length > this._limit)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var buffer = this.b.slice(this._position, this._position + length);
        this._position += length;
        return new Uint8Array(buffer);
    },
    getArrayAt: function(position, length)
    {
        if(position + length > this._limit)
        {
            throw new Error(__BufferUnderflowException__);
        }
        length = length === undefined ? (this.b.byteLength - position) : length;
        return new Uint8Array(this.b.slice(position, position + length));
    },
    getShort: function()
    {
        if(this._limit - this._position < 2)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.v.getInt16(this._position, true);
        this._position += 2;
        return v;
    },
    getInt: function()
    {
        if(this._limit - this._position < 4)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.v.getInt32(this._position, true);
        this._position += 4;
        return v;
    },
    getFloat: function()
    {
        if(this._limit - this._position < 4)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.v.getFloat32(this._position, true);
        this._position += 4;
        return v;
    },
    getDouble: function()
    {
        if(this._limit - this._position < 8)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.v.getFloat64(this._position, true);
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
        v.low = this.v.getUint32(this._position, true);
        this._position += 4;
        v.high = this.v.getUint32(this._position, true);
        this._position += 4;
        return v;
    },
    getString: function(length)
    {
        if(this._position + length > this._limit)
        {
            throw new Error(__BufferUnderflowException__);
        }

        var data = new DataView(this.b, this._position, length);
        var s = "";

        for(var i = 0; i < length; ++i)
        {
            s += String.fromCharCode(data.getUint8(i));
        }
        this._position += length;
        s = decodeURIComponent(escape(s));
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
    get: function() { return this.b === null ? 0 : this.b.byteLength; }
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
        return new Uint8Array(0);
    }
    else
    {
        return new Uint8Array(data);
    }
};

Ice.Buffer = Buffer;
