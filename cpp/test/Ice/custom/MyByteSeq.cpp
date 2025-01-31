// Copyright (c) ZeroC, Inc.

#include "MyByteSeq.h"
#include <cstring>

using namespace std;

MyByteSeq::MyByteSeq() : _size(0), _data(nullptr) {}

MyByteSeq::MyByteSeq(size_t size) : _size(size), _data(nullptr)
{
    if (_size != 0)
    {
        _data = new std::byte[_size];
    }
}

MyByteSeq::MyByteSeq(const MyByteSeq& seq)
{
    _size = seq._size;
    if (_size != 0)
    {
        _data = new std::byte[_size];
        memcpy(_data, seq._data, _size);
    }
    else
    {
        _data = nullptr;
    }
}

MyByteSeq::MyByteSeq(MyByteSeq&& seq) noexcept : _size(seq._size), _data(seq._data)
{
    seq._size = 0;
    seq._data = nullptr;
}

MyByteSeq::~MyByteSeq() { delete[] _data; }

size_t
MyByteSeq::size() const
{
    return _size;
}

void
MyByteSeq::swap(MyByteSeq& seq) noexcept
{
    size_t tmpSize = seq._size;
    std::byte* tmpData = seq._data;
    seq._size = _size;
    seq._data = _data;
    _size = tmpSize;
    _data = tmpData;
}

MyByteSeq::const_iterator
MyByteSeq::begin() const
{
    return _data;
}

MyByteSeq::const_iterator
MyByteSeq::end() const
{
    return _data + _size;
}

MyByteSeq&
MyByteSeq::operator=(const MyByteSeq& rhs)
{
    if (this != &rhs)
    {
        delete[] _data;
        _data = nullptr;

        _size = rhs._size;
        if (_size != 0)
        {
            _data = new std::byte[_size];
            memcpy(_data, rhs._data, _size);
        }
    }
    return *this;
}

MyByteSeq&
MyByteSeq::operator=(MyByteSeq&& rhs) noexcept
{
    delete[] _data;
    _data = nullptr;

    _size = rhs._size;
    _data = rhs._data;

    rhs._size = 0;
    rhs._data = nullptr;
    return *this;
}

bool
MyByteSeq::operator==(const MyByteSeq& rhs) const
{
    if (_size != rhs._size)
    {
        return _size == rhs._size;
    }
    return memcmp(_data, rhs._data, _size) == 0;
}
