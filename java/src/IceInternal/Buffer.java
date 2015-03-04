// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

//
// An instance of java.nio.ByteBuffer cannot grow beyond its initial capacity.
// This class wraps a ByteBuffer and supports reallocation.
//
public class Buffer
{
    public
    Buffer(int maxCapacity, boolean direct)
    {
        b = _emptyBuffer;
        _size = 0;
        _capacity = 0;
        _maxCapacity = maxCapacity;
        _direct = direct;
    }

    public
    Buffer(byte[] data)
    {
        b = java.nio.ByteBuffer.wrap(data);
        b.order(java.nio.ByteOrder.LITTLE_ENDIAN);
        _size = data.length;
        _capacity = 0;
        _maxCapacity = 0;
        _direct = false;
    }

    public
    Buffer(java.nio.ByteBuffer data)
    {
        b = data;
        b.order(java.nio.ByteOrder.LITTLE_ENDIAN);
        _size = data.remaining();
        _capacity = 0;
        _maxCapacity = 0;
        _direct = false;
    }

    public int
    size()
    {
        return _size;
    }

    public boolean
    empty()
    {
        return _size == 0;
    }

    public void
    clear()
    {
        b = _emptyBuffer;
        _size = 0;
        _capacity = 0;
    }

    //
    // Call expand(n) to add room for n additional bytes. Note that expand()
    // examines the current position of the buffer first; we don't want to
    // expand the buffer if the caller is writing to a location that is
    // already in the buffer.
    //
    public void
    expand(int n)
    {
        final int sz = (b == _emptyBuffer) ? n : b.position() + n;
        if(sz > _size)
        {
            resize(sz, false);
        }
    }

    public void
    resize(int n, boolean reading)
    {
        assert(b == _emptyBuffer || _capacity > 0);

        if(n == 0)
        {
            clear();
        }
        else if(n > _capacity)
        {
            reserve(n);
        }
        _size = n;

        //
        // When used for reading, we want to set the buffer's limit to the new size.
        //
        if(reading)
        {
            b.limit(_size);
        }
    }

    public void
    reset()
    {
        if(_size > 0 && _size * 2 < _capacity)
        {
            //
            // If the current buffer size is smaller than the
            // buffer capacity, we shrink the buffer memory to the
            // current size. This is to avoid holding on to too much
            // memory if it's not needed anymore.
            //
            if(++_shrinkCounter > 2)
            {
                reserve(_size);
                _shrinkCounter = 0;
            }
        }
        else
        {
            _shrinkCounter = 0;
        }
        _size = 0;
        if(b != _emptyBuffer)
        {
            b.limit(b.capacity());
            b.position(0);
        }
    }

    private void
    reserve(int n)
    {
        if(n > _capacity)
        {
            _capacity = java.lang.Math.max(n, java.lang.Math.min(2 * _capacity, _maxCapacity));
            _capacity = java.lang.Math.max(240, _capacity);
        }
        else if(n < _capacity)
        {
            _capacity = n;
        }
        else
        {
            return;
        }

        try
        {
            java.nio.ByteBuffer buf;

            if(_direct)
            {
                buf = java.nio.ByteBuffer.allocateDirect(_capacity);
            }
            else
            {
                buf = java.nio.ByteBuffer.allocate(_capacity);
            }

            if(b == _emptyBuffer)
            {
                b = buf;
            }
            else
            {
                final int pos = b.position();
                b.position(0);
                b.limit(java.lang.Math.min(_capacity, b.capacity()));
                buf.put(b);
                b = buf;
                b.limit(b.capacity());
                b.position(pos);
            }

            b.order(java.nio.ByteOrder.LITTLE_ENDIAN);
        }
        catch(OutOfMemoryError ex)
        {
            _capacity = b.capacity(); // Restore the previous capacity.
            throw ex;
        }
    }

    public java.nio.ByteBuffer b;
    // Sentinel used for null buffer.
    public java.nio.ByteBuffer _emptyBuffer = java.nio.ByteBuffer.allocate(0);

    private int _size;
    private int _capacity; // Cache capacity to avoid excessive method calls.
    private int _maxCapacity;
    private boolean _direct; // Use direct buffers?
    private int _shrinkCounter;
}
