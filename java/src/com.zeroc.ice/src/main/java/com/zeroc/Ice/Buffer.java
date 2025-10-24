// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * An instance of {@link java.nio.ByteBuffer} cannot grow beyond its initial capacity.
 * This class wraps a {@link ByteBuffer} and supports reallocation.
 *
 * @hidden Public because it's used by IceBT, SSL, and the 'Ice/background' test.
 */
public class Buffer {
    public Buffer(boolean direct) {
        this(direct, ByteOrder.LITTLE_ENDIAN);
    }

    public Buffer(boolean direct, ByteOrder order) {
        b = _emptyBuffer;
        _size = 0;
        _capacity = 0;
        _direct = direct;
        _order = order;
    }

    public Buffer(byte[] data) {
        this(data, ByteOrder.LITTLE_ENDIAN);
    }

    public Buffer(byte[] data, ByteOrder order) {
        b = ByteBuffer.wrap(data);
        b.order(order);
        _size = data.length;
        _capacity = 0;
        _direct = false;
        _order = order;
    }

    public Buffer(ByteBuffer data) {
        this(data, ByteOrder.LITTLE_ENDIAN);
    }

    public Buffer(ByteBuffer data, ByteOrder order) {
        b = data;
        b.order(order);
        _size = data.remaining();
        _capacity = 0;
        _direct = false;
        _order = order;
    }

    public Buffer(Buffer buf, boolean adopt) {
        b = buf.b;
        _size = buf._size;
        _capacity = buf._capacity;
        _direct = buf._direct;
        _shrinkCounter = buf._shrinkCounter;
        _order = buf._order;

        if (adopt) {
            buf.clear();
        }
    }

    public java.nio.Buffer position(int newPosition) {
        // Cast to java.nio.Buffer to avoid incompatible covariant return type used by java.nio.ByteBuffer.
        return ((java.nio.Buffer) b).position(newPosition);
    }

    public java.nio.Buffer limit(int newLimit) {
        // Cast to java.nio.Buffer to avoid incompatible covariant return type used by java.nio.ByteBuffer.
        return ((java.nio.Buffer) b).limit(newLimit);
    }

    public java.nio.Buffer flip() {
        // Cast to java.nio.Buffer to avoid incompatible covariant return type used by java.nio.ByteBuffer.
        return ((java.nio.Buffer) b).flip();
    }

    public void swap(Buffer buf) {
        final ByteBuffer bb = buf.b;
        final int size = buf._size;
        final int capacity = buf._capacity;
        final boolean direct = buf._direct;
        final int shrinkCounter = buf._shrinkCounter;
        final ByteOrder order = buf._order;

        buf.b = b;
        buf._size = _size;
        buf._capacity = _capacity;
        buf._direct = _direct;
        buf._shrinkCounter = _shrinkCounter;
        buf._order = _order;

        b = bb;
        _size = size;
        _capacity = capacity;
        _direct = direct;
        _shrinkCounter = shrinkCounter;
        _order = order;
    }

    public int size() {
        return _size;
    }

    public boolean empty() {
        return _size == 0;
    }

    public void clear() {
        b = _emptyBuffer;
        _size = 0;
        _capacity = 0;
        _shrinkCounter = 0;
    }

    /**
     * Adds room to this buffer for {@code n} additional bytes.
     * Note that {@code expand} examines the current position of the buffer first;
     * we don't want to expand the buffer if the caller is writing to a location that is already in the buffer.
     *
     * @param n the number of additional bytes
     */
    public void expand(int n) {
        final int sz = b == _emptyBuffer ? n : b.position() + n;
        if (sz > _size) {
            resize(sz, false);
        }
    }

    public void resize(int n, boolean reading) {
        assert (b == _emptyBuffer || _capacity > 0);

        if (n == 0) {
            clear();
        } else if (n > _capacity) {
            reserve(n);
        }
        _size = n;

        // When used for reading, we want to set the buffer's limit to the new size.
        if (reading) {
            limit(_size);
        }
    }

    public void reset() {
        if (_size > 0 && _size * 2 < _capacity) {
            // If the current buffer size is smaller than the buffer capacity, we shrink the buffer memory
            // to the current size. This is to avoid holding on to too much memory if it's not needed anymore.
            if (++_shrinkCounter > 2) {
                reserve(_size);
                _shrinkCounter = 0;
            }
        } else {
            _shrinkCounter = 0;
        }
        _size = 0;
        if (b != _emptyBuffer) {
            limit(b.capacity());
            position(0);
        }
    }

    private void reserve(int n) {
        if (n > _capacity) {
            _capacity = java.lang.Math.max(n, 2 * _capacity);
            _capacity = java.lang.Math.max(240, _capacity);
        } else if (n < _capacity) {
            _capacity = n;
        } else {
            return;
        }

        try {
            ByteBuffer buf;

            if (_direct) {
                buf = ByteBuffer.allocateDirect(_capacity);
            } else {
                buf = ByteBuffer.allocate(_capacity);
            }

            if (b == _emptyBuffer) {
                b = buf;
            } else {
                final int pos = b.position();
                position(0);
                limit(java.lang.Math.min(_capacity, b.capacity()));
                buf.put(b);
                b = buf;
                limit(b.capacity());
                position(pos);
            }

            b.order(_order); // Preserve the original order.
        } catch (OutOfMemoryError ex) {
            _capacity = b.capacity(); // Restore the previous capacity.
            throw ex;
        }
    }

    public ByteBuffer b;
    // Sentinel used for null buffer.
    public ByteBuffer _emptyBuffer = ByteBuffer.allocate(0);

    private int _size;
    private int _capacity; // Cache capacity to avoid excessive method calls.
    private boolean _direct; // Use direct buffers?
    private int _shrinkCounter;
    private ByteOrder _order;
}
