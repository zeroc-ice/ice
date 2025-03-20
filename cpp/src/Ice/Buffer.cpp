// Copyright (c) ZeroC, Inc.

#include "Ice/Buffer.h"

#include <cstdlib>
#include <cstring>

using namespace std;
using namespace IceInternal;

IceInternal::Buffer::Container::Container() noexcept
    : _buf(nullptr),
      _size(0),
      _capacity(0),
      _shrinkCounter(0),
      _owned(true)
{
}

IceInternal::Buffer::Container::Container(const_iterator beg, const_iterator end) noexcept
    : _buf(const_cast<iterator>(beg)),
      _size(static_cast<size_t>(end - beg)),
      _capacity(static_cast<size_t>(end - beg)),
      _shrinkCounter(0),
      _owned(false)
{
}

IceInternal::Buffer::Container::Container(const vector<value_type>& v) noexcept : _shrinkCounter(0)
{
    if (v.empty())
    {
        _buf = nullptr;
        _size = 0;
        _capacity = 0;
        _owned = true;
    }
    else
    {
        _buf = const_cast<value_type*>(&v[0]);
        _size = v.size();
        _capacity = _size;
        _owned = false;
    }
}

IceInternal::Buffer::Container::Container(Container& other, bool adopt) noexcept
{
    if (adopt)
    {
        _buf = other._buf;
        _size = other._size;
        _capacity = other._capacity;
        _shrinkCounter = other._shrinkCounter;
        _owned = other._owned;

        other._buf = nullptr;
        other._size = 0;
        other._capacity = 0;
        other._shrinkCounter = 0;
        other._owned = true;
    }
    else
    {
        _buf = other._buf;
        _size = other._size;
        _capacity = other._capacity;
        _shrinkCounter = 0;
        _owned = false;
    }
}

IceInternal::Buffer::Container::Container(Container&& other) noexcept
    : _buf(other._buf),
      _size(other._size),
      _capacity(other._capacity),
      _shrinkCounter(other._shrinkCounter),
      _owned(other._owned)
{
    // Reset other to default state.
    other._buf = nullptr;
    other._size = 0;
    other._capacity = 0;
    other._shrinkCounter = 0;
    other._owned = true;
}

IceInternal::Buffer::Container&
IceInternal::Buffer::Container::operator=(Container&& other) noexcept
{
    if (this != &other)
    {
        // If we own the buffer, free it first.
        if (_owned)
        {
            ::free(_buf);
        }
        _buf = other._buf;
        _size = other._size;
        _capacity = other._capacity;
        _shrinkCounter = other._shrinkCounter;
        _owned = other._owned;

        // Reset other to default state.
        other._buf = nullptr;
        other._size = 0;
        other._capacity = 0;
        other._shrinkCounter = 0;
        other._owned = true;
    }
    return *this;
}

IceInternal::Buffer::Container::~Container()
{
    if (_owned)
    {
        ::free(_buf);
    }
}

void
IceInternal::Buffer::Container::swap(Container& other) noexcept
{
    std::swap(_buf, other._buf);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(_shrinkCounter, other._shrinkCounter);
    std::swap(_owned, other._owned);
}

void
IceInternal::Buffer::Container::clear()
{
    if (_owned)
    {
        ::free(_buf);
    }

    _buf = nullptr;
    _size = 0;
    _capacity = 0;
    _shrinkCounter = 0;
    _owned = true;
}

void
IceInternal::Buffer::Container::reserve(size_type n)
{
    size_type c = _capacity;
    if (n > _capacity)
    {
        _capacity = std::max<size_type>(n, 2 * _capacity);
        _capacity = std::max<size_type>(static_cast<size_type>(240), _capacity);
    }
    else if (n < _capacity)
    {
        _capacity = n;
    }
    else
    {
        return;
    }

    pointer p;
    if (_owned)
    {
        p = reinterpret_cast<pointer>(::realloc(_buf, _capacity));
    }
    else
    {
        p = reinterpret_cast<pointer>(::malloc(_capacity));
        if (p)
        {
            ::memcpy(p, _buf, _size);
            _owned = true;
        }
    }

    if (!p)
    {
        _capacity = c; // Restore the previous capacity.
        throw std::bad_alloc();
    }

    _buf = p;
}
