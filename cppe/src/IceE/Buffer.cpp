// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Buffer.h>
#include <IceE/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::Buffer::swap(Buffer& other)
{
#ifdef ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION
    Container::difference_type pos = i - b.begin();
    Container::difference_type otherPos = other.i - other.b.begin();
    b.swap(other.b);
    i = b.begin() + otherPos;
    other.i = other.b.begin() + pos;
#else
    b.swap(other.b);
    std::swap(i, other.i); 
#endif
}

void
IceInternal::Buffer::Container::swap(Container& other)
{
#ifdef ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION
    if(_buf == _fixed)
    {
	if(other._buf == other._fixed)
	{
	    value_type tmp[ICE_BUFFER_FIXED_SIZE];
	    memcpy(tmp, _fixed, _size);
	    memcpy(_fixed, other._fixed, other._size);
	    memcpy(other._fixed, tmp, _size);
	}
	else
	{
	    _buf = other._buf;
	    memcpy(other._fixed, _fixed, _size);
	    other._buf = other._fixed;
	}
    }
    else
    {
	if(other._buf == other._fixed)
	{
	    other._buf = _buf;
	    memcpy(_fixed, other._fixed, other._size);
	    _buf = _fixed;
	}
	else
	{
	    std::swap(_buf, other._buf);
	}
    }
#else
    std::swap(_buf, other._buf);
#endif

    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(_shrinkCounter, other._shrinkCounter);
}

void
IceInternal::Buffer::Container::clear()
{
#ifdef ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION
    if(_buf != _fixed)
    {
	free(_buf);
	_buf = _fixed;
    }
    _size = 0;
    _capacity = ICE_BUFFER_FIXED_SIZE;
#else
    free(_buf);
    _buf = 0;
    _size = 0;
    _capacity = 0;
#endif
}

void
IceInternal::Buffer::Container::reserve(size_type n)
{
    if(n > _capacity)
    {
	_capacity = std::max<size_type>(n, std::min(2 * _capacity, _maxCapacity));
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
    
#ifdef ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION
    if(_buf != _fixed)
    {
	_buf = reinterpret_cast<pointer>(realloc(_buf, _capacity));
    }
    else if(_capacity > ICE_BUFFER_FIXED_SIZE)
    {
	_buf = reinterpret_cast<pointer>(malloc(_capacity));
	memcpy(_buf, _fixed, _size);
    }
#else
    if(_buf)
    {
	_buf = reinterpret_cast<pointer>(realloc(_buf, _capacity));
    }
    else
    {
	_buf = reinterpret_cast<pointer>(malloc(_capacity));
    }
#endif
	
    if(!_buf)
    {
	SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
}
