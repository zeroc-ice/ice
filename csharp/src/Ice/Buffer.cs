//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;

namespace IceInternal
{
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
            B = _emptyBuffer;
            _size = 0;
            _capacity = 0;
            _order = order;
        }

        public Buffer(byte[] data) : this(data, ByteBuffer.ByteOrder.LITTLE_ENDIAN)
        {
        }

        public Buffer(byte[] data, ByteBuffer.ByteOrder order)
        {
            B = ByteBuffer.Wrap(data);
            B.Order(order);
            _size = data.Length;
            _capacity = 0;
            _order = order;
        }

        public Buffer(ByteBuffer data) : this(data, ByteBuffer.ByteOrder.LITTLE_ENDIAN)
        {
        }

        public Buffer(ByteBuffer data, ByteBuffer.ByteOrder order)
        {
            B = data;
            B.Order(order);
            _size = data.Remaining();
            _capacity = 0;
            _order = order;
        }

        public Buffer(Buffer buf, bool adopt)
        {
            B = buf.B;
            _size = buf._size;
            _capacity = buf._capacity;
            _shrinkCounter = buf._shrinkCounter;
            _order = buf._order;

            if (adopt)
            {
                buf.Clear();
            }
        }

        public int Size() => _size;

        public bool Empty() => _size == 0;

        public void Clear()
        {
            B = _emptyBuffer;
            _size = 0;
            _capacity = 0;
            _shrinkCounter = 0;
        }

        //
        // Call expand(n) to add room for n additional bytes. Note that expand()
        // examines the current position of the buffer first; we don't want to
        // expand the buffer if the caller is writing to a location that is
        // already in the buffer.
        //
        public void Expand(int n)
        {
            int sz = (B == _emptyBuffer) ? n : B.Position() + n;
            if (sz > _size)
            {
                Resize(sz, false);
            }
        }

        public void Resize(int n, bool reading)
        {
            Debug.Assert(B == _emptyBuffer || _capacity > 0);

            if (n == 0)
            {
                Clear();
            }
            else if (n > _capacity)
            {
                Reserve(n);
            }
            _size = n;

            //
            // When used for reading, we want to set the buffer's limit to the new size.
            //
            if (reading)
            {
                B.Limit(_size);
            }
        }

        public void Reset()
        {
            if (_size > 0 && _size * 2 < _capacity)
            {
                //
                // If the current buffer size is smaller than the
                // buffer capacity, we shrink the buffer memory to the
                // current size. This is to avoid holding on to too much
                // memory if it's not needed anymore.
                //
                if (++_shrinkCounter > 2)
                {
                    Reserve(_size);
                    _shrinkCounter = 0;
                }
            }
            else
            {
                _shrinkCounter = 0;
            }
            _size = 0;
            if (B != _emptyBuffer)
            {
                B.Limit(B.Capacity());
                B.Position(0);
            }
        }

        private void Reserve(int n)
        {
            Debug.Assert(_capacity == B.Capacity());

            if (n > _capacity)
            {
                _capacity = System.Math.Max(n, 2 * _capacity);
                _capacity = System.Math.Max(240, _capacity);
            }
            else if (n < _capacity)
            {
                _capacity = n;
            }
            else
            {
                return;
            }

            try
            {
                var buf = ByteBuffer.Allocate(_capacity);

                if (B == _emptyBuffer)
                {
                    B = buf;
                }
                else
                {
                    int pos = B.Position();
                    B.Position(0);
                    B.Limit(System.Math.Min(_capacity, B.Capacity()));
                    buf.Put(B);
                    B = buf;
                    B.Limit(B.Capacity());
                    B.Position(pos);
                }

                B.Order(_order);
            }
            catch (System.OutOfMemoryException)
            {
                _capacity = B.Capacity(); // Restore the previous capacity
                throw;
            }
            catch (System.Exception ex)
            {
                _capacity = B.Capacity(); // Restore the previous capacity.
                throw new Ice.MarshalException(
                    $"unexpected exception while trying to allocate a ByteBuffer:\n{ex}", ex);
            }
            finally
            {
                Debug.Assert(_capacity == B.Capacity());
            }
        }

        public ByteBuffer B;
        // Sentinel used for null buffer.
        private static readonly ByteBuffer _emptyBuffer = new ByteBuffer();

        private int _size;
        private int _capacity; // Cache capacity to avoid excessive method calls.
        private int _shrinkCounter;
        private readonly ByteBuffer.ByteOrder _order;
    }

}
