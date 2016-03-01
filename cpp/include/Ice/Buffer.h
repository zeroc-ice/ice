// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BUFFER_H
#define ICE_BUFFER_H

#include <Ice/Config.h>

namespace IceInternal
{

class ICE_API Buffer : private IceUtil::noncopyable
{
public:

    Buffer() : i(b.begin()) { }
    Buffer(const Ice::Byte* beg, const Ice::Byte* end) : b(beg, end), i(b.begin()) { }
    virtual ~Buffer() { }

    void swapBuffer(Buffer&);

    class ICE_API Container : private IceUtil::noncopyable
    {
    public:

        //
        // Standard vector-like operations.
        //

        typedef Ice::Byte value_type;
        typedef Ice::Byte* iterator;
        typedef const Ice::Byte* const_iterator;
        typedef Ice::Byte& reference;
        typedef const Ice::Byte& const_reference;
        typedef Ice::Byte* pointer;
        typedef size_t size_type;

        Container();
        Container(const_iterator, const_iterator);

        ~Container();

        iterator begin()
        {
            return _buf;
        }

        const_iterator begin() const
        {
            return _buf;
        }

        iterator end()
        {
            return _buf + _size;
        }

        const_iterator end() const
        {
            return _buf + _size;
        }

        size_type size() const
        {
            return _size;
        }

        bool empty() const
        {
            return !_size;
        }

        void swap(Container&);
        
        void clear();

        void resize(size_type n) // Inlined for performance reasons.
        {
            assert(!_buf || _capacity > 0);

            if(n == 0)
            {
                clear();
            }
            else if(n > _capacity)
            {
                reserve(n);
            }
            _size = n;
        }
        
        void reset()
        {
            assert(!_buf || _capacity > 0);

            if(_size > 0 && _size * 2 < _capacity)
            {
                //
                // If the current buffer size is smaller than the
                // buffer capacity, we shrink the buffer memory to the
                // current size. This is to avoid holding on too much
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
        }

        void push_back(value_type v)
        {
            resize(_size + 1);
            _buf[_size - 1] = v;
        }

        reference operator[](size_type n)
        {
            assert(n < _size);
            return _buf[n];
        }

        const_reference operator[](size_type n) const
        {
            assert(n < _size);
            return _buf[n];
        }
        
    private:

        Container(const Container&);
        void operator=(const Container&);
        void reserve(size_type);

        pointer _buf;
        size_type _size;
        size_type _capacity;
        int _shrinkCounter;
    };

    Container b;
    Container::iterator i;
};

}

#endif
