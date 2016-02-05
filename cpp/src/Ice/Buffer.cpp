// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Buffer.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::Buffer::swapBuffer(Buffer& other)
{
    b.swap(other.b);
    std::swap(i, other.i);
}

IceInternal::Buffer::Container::Container() :
    _buf(0),
    _size(0),
    _capacity(0),
    _shrinkCounter(0)
{
}

IceInternal::Buffer::Container::Container(const_iterator beg, const_iterator end) :
    _buf(const_cast<iterator>(beg)),
    _size(end - beg),
    _capacity(0),
    _shrinkCounter(0)
{
}

IceInternal::Buffer::Container::Container(const vector<value_type>& v) :
    _capacity(0),
    _shrinkCounter(0)
{
    if(v.empty())
    {
        _buf = 0;
        _size = 0;
    }
    else
    {
        _buf = const_cast<value_type*>(&v[0]);
        _size = v.size();
    }
}

IceInternal::Buffer::Container::Container(Container& other, bool adopt)
{
    if(adopt)
    {
        _buf = other._buf;
        _size = other._size;
        _capacity = other._capacity;
        _shrinkCounter = other._shrinkCounter;

        other._buf = 0;
        other._size = 0;
        other._capacity = 0;
        other._shrinkCounter = 0;
    }
    else
    {
        _buf = other._buf;
        _size = other._size;
        _capacity = 0;
        _shrinkCounter = 0;
    }
}

IceInternal::Buffer::Container::~Container()
{
    if(_buf && _capacity > 0)
    {
        ::free(_buf);
    }
}

void
IceInternal::Buffer::Container::swap(Container& other)
{
    std::swap(_buf, other._buf);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(_shrinkCounter, other._shrinkCounter);
}

void
IceInternal::Buffer::Container::clear()
{
    if(_buf && _capacity > 0)
    {
        free(_buf);
    }
    _buf = 0;
    _size = 0;
    _capacity = 0;
}

void
IceInternal::Buffer::Container::reserve(size_type n)
{
    assert(!_buf || _capacity > 0);

    size_type c = _capacity;
    if(n > _capacity)
    {
        _capacity = std::max<size_type>(n, 2 * _capacity);
        _capacity = std::max<size_type>(static_cast<size_type>(240), _capacity);
    }
    else if(n < _capacity)
    {
        _capacity = n;
    }
    else
    {
        return;
    }

    pointer p = reinterpret_cast<pointer>(::realloc(_buf, _capacity));
    if(!p)
    {
        _capacity = c; // Restore the previous capacity.
        throw std::bad_alloc();
    }
    _buf = p;
}
