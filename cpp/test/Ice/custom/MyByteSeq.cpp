// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <MyByteSeq.h>

MyByteSeq::MyByteSeq()
    : _size(0),
      _data(0)
{
}

MyByteSeq::MyByteSeq(size_t size)
    : _size(size),
      _data(0)
{
    if(_size != 0)
    {
        _data = new Ice::Byte[_size];
    }
}

MyByteSeq::MyByteSeq(const MyByteSeq& seq)
{
    _size = seq._size;
    if(_size != 0)
    {
        _data = new Ice::Byte[_size];
        memcpy(_data, seq._data, _size);
    }
    else
    {
        _data = 0;
    }
}

MyByteSeq::~MyByteSeq()
{
    delete[] _data;
}

size_t
MyByteSeq::size() const
{
    return _size;
}

void
MyByteSeq::swap(MyByteSeq& seq)
{
    size_t tmpSize = seq._size;
    Ice::Byte* tmpData = seq._data;
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

void
MyByteSeq::operator=(const MyByteSeq& rhs)
{
    delete[] _data;
    _data = 0;

    _size = rhs._size;
    if(_size != 0)
    {
        _data = new Ice::Byte[_size];
        memcpy(_data, rhs._data, _size);
    }
}

bool
MyByteSeq::operator==(const MyByteSeq& rhs) const
{
    if(_size != rhs._size)
    {
        return _size == rhs._size;
    }
    return memcmp(_data, rhs._data, _size) == 0;
}
