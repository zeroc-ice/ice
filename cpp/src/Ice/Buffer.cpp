// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Buffer.h>
#include <Ice/LocalException.h>
#include <Ice/MemoryPool.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::Buffer::swap(Buffer& other)
{
    b.swap(other.b);
    std::swap(i, other.i); 
}

IceInternal::Buffer::Container::Container(IceInternal::MemoryPool* pool) :
    _buf(0),
    _size(0),
    _capacity(0),
    _pool(pool)
{
}

IceInternal::Buffer::Container::~Container()
{
    if(_pool)
    {
	_pool->free(_buf);
    }
    else
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
    if(_pool)
    {
	_pool->free(_buf);
    }
    else
    {
	::free(_buf);
    }
    _buf = 0;
    _size = 0;
    _capacity = 0;
}

void
IceInternal::Buffer::Container::reserve(size_type n)
{
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
    
    if(_buf)
    {
	if(_pool)
	{
	    _buf = _pool->realloc(_buf, _capacity);
	}
	else
	{
	    _buf = reinterpret_cast<pointer>(::realloc(_buf, _capacity));
	}
    }
    else
    {
	if(_pool)
	{
	    _buf = _pool->alloc(_capacity);
	}
	else
	{
	    _buf = reinterpret_cast<pointer>(::malloc(_capacity));
	}
    }
	
    if(!_buf)
    {
	SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
}
