// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/Long").Ice;
const Long = Ice.Long;

const __BufferOverflowException__ = "BufferOverflowException";
const __BufferUnderflowException__ = "BufferUnderflowException";
const __IndexOutOfBoundsException__ = "IndexOutOfBoundsException";

//
// Buffer implementation to be used by web browsers, it uses ArrayBuffer as
// the store.
//
class Buffer
{
    constructor(buffer)
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
    }

    empty()
    {
        return this._limit === 0;
    }

    resize(n)
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
    }

    clear()
    {
        this.b = null;
        this.v = null;
        this._position = 0;
        this._limit = 0;
    }

    //
    // Call expand(n) to add room for n additional bytes. Note that expand()
    // examines the current position of the buffer first; we don't want to
    // expand the buffer if the caller is writing to a location that is
    // already in the buffer.
    //
    expand(n)
    {
        var sz = this.capacity === 0 ? n : this._position + n;
        if(sz > this._limit)
        {
            this.resize(sz);
        }
    }

    reset()
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
    }

    reserve(n)
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
    }

    put(v)
    {
        if(this._position === this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setUint8(this._position, v);
        this._position++;
    }

    putAt(i, v)
    {
        if(i >= this._limit)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        this.v.setUint8(i, v);
    }

    putArray(v)
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
    }

    putShort(v)
    {
        if(this._position + 2 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setInt16(this._position, v, true);
        this._position += 2;
    }

    putInt(v)
    {
        if(this._position + 4 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setInt32(this._position, v, true);
        this._position += 4;
    }

    putIntAt(i, v)
    {
        if(i + 4 > this._limit || i < 0)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        this.v.setInt32(i, v, true);
    }

    putFloat(v)
    {
        if(this._position + 4 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setFloat32(this._position, v, true);
        this._position += 4;
    }

    putDouble(v)
    {
        if(this._position + 8 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setFloat64(this._position, v, true);
        this._position += 8;
    }

    putLong(v)
    {
        if(this._position + 8 > this._limit)
        {
            throw new Error(__BufferOverflowException__);
        }
        this.v.setInt32(this._position, v.low, true);
        this._position += 4;
        this.v.setInt32(this._position, v.high, true);
        this._position += 4;
    }

    writeString(stream, v)
    {
        //
        // Encode the string as utf8
        //
        var encoded = unescape(encodeURIComponent(v));

        stream.writeSize(encoded.length);
        stream.expand(encoded.length);
        this.putString(encoded, encoded.length);
    }

    putString(v, sz)
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
    }

    get()
    {
        if(this._position >= this._limit)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.v.getUint8(this._position);
        this._position++;
        return v;
    }

    getAt(i)
    {
        if(i < 0 || i >= this._limit)
        {
            throw new Error(__IndexOutOfBoundsException__);
        }
        return this.v.getUint8(i);
    }

    getArray(length)
    {
        if(this._position + length > this._limit)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var buffer = this.b.slice(this._position, this._position + length);
        this._position += length;
        return new Uint8Array(buffer);
    }

    getArrayAt(position, length)
    {
        if(position + length > this._limit)
        {
            throw new Error(__BufferUnderflowException__);
        }
        length = length === undefined ? (this.b.byteLength - position) : length;
        return new Uint8Array(this.b.slice(position, position + length));
    }

    getShort()
    {
        if(this._limit - this._position < 2)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.v.getInt16(this._position, true);
        this._position += 2;
        return v;
    }

    getInt()
    {
        if(this._limit - this._position < 4)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.v.getInt32(this._position, true);
        this._position += 4;
        return v;
    }

    getFloat()
    {
        if(this._limit - this._position < 4)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.v.getFloat32(this._position, true);
        this._position += 4;
        return v;
    }

    getDouble()
    {
        if(this._limit - this._position < 8)
        {
            throw new Error(__BufferUnderflowException__);
        }
        var v = this.v.getFloat64(this._position, true);
        this._position += 8;
        return v;
    }

    getLong()
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
    }

    getString(length)
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
    
    get position()
    {
        return this._position;
    }
    
    set position(value)
    {
        if(value >= 0 && value <= this._limit)
        {
            this._position = value;
        }
    }
    
    get limit()
    {
        return this._limit;
    }
    
    set limit(value)
    {
        if(value <= this.capacity)
        {
            this._limit = value;
            if(this._position > value)
            {
                this._position = value;
            }
        }
    }
    
    get capacity()
    {
        return this.b === null ? 0 : this.b.byteLength;
    }
    
    get remaining()
    {
        return this._limit - this._position;
    }
    
    //
    // Create a native buffer from an array of bytes.
    //
    static createNative(data)
    {
        if(data === undefined)
        {
            return new Uint8Array(0);
        }
        else
        {
            return new Uint8Array(data);
        }
    }
}

Ice.Buffer = Buffer;
module.exports.Ice = Ice;