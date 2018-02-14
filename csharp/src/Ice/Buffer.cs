// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Diagnostics;

    //
    // An instance of ByteBuffer cannot grow beyond its initial capacity.
    // This class wraps a ByteBuffer and supports reallocation.
    //
    public class Buffer
    {
        public Buffer() : this(ByteBuffer.ByteOrder.LITTLE_ENDIAN)
        {
        }

        public Buffer(ByteBuffer.ByteOrder order)
        {
            b = _emptyBuffer;
            _size = 0;
            _capacity = 0;
            _order = order;
        }

        public Buffer(byte[] data) : this(data, ByteBuffer.ByteOrder.LITTLE_ENDIAN)
        {
        }

        public Buffer(byte[] data, ByteBuffer.ByteOrder order)
        {
            b = ByteBuffer.wrap(data);
            b.order(order);
            _size = data.Length;
            _capacity = 0;
            _order = order;
        }

        public int size()
        {
            return _size;
        }

        public bool empty()
        {
            return _size == 0;
        }

        public void clear()
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
        public void expand(int n)
        {
            int sz = (b == _emptyBuffer) ? n : b.position() + n;
            if(sz > _size)
            {
                resize(sz, false);
            }
        }

        public void resize(int n, bool reading)
        {
            Debug.Assert(b == _emptyBuffer || _capacity > 0);

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

        public void reset()
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

        private void reserve(int n)
        {
            Debug.Assert(_capacity == b.capacity());

            if(n > _capacity)
            {
                _capacity = System.Math.Max(n, 2 * _capacity);
                _capacity = System.Math.Max(240, _capacity);
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
                ByteBuffer buf = ByteBuffer.allocate(_capacity);

                if(b == _emptyBuffer)
                {
                    b = buf;
                }
                else
                {
                    int pos = b.position();
                    b.position(0);
                    b.limit(System.Math.Min(_capacity, b.capacity()));
                    buf.put(b);
                    b = buf;
                    b.limit(b.capacity());
                    b.position(pos);
                }

                b.order(_order);
            }
            catch(System.OutOfMemoryException)
            {
                _capacity = b.capacity(); // Restore the previous capacity
                throw;
            }
            catch(System.Exception ex)
            {
                _capacity = b.capacity(); // Restore the previous capacity.
                Ice.MarshalException e = new Ice.MarshalException(ex);
                e.reason = "unexpected exception while trying to allocate a ByteBuffer:\n" + ex;
                throw e;
            }
            finally
            {
                Debug.Assert(_capacity == b.capacity());
            }
        }

        public ByteBuffer b;
        // Sentinel used for null buffer.
        static private ByteBuffer _emptyBuffer = new ByteBuffer();

        private int _size;
        private int _capacity; // Cache capacity to avoid excessive method calls.
        private int _shrinkCounter;
        private ByteBuffer.ByteOrder _order;
    }

}
