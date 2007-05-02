// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_BUFFER_H
#define ICEE_BUFFER_H

#include <IceE/Config.h>
#include <cstdlib>

#define ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION
#define ICE_BUFFER_FIXED_SIZE 64

namespace IceInternal
{

class Buffer : private IceUtil::noncopyable
{
public:

    Buffer(size_t maxCapacity) : b(maxCapacity), i(b.begin()) { }
    virtual ~Buffer() { }

    ICE_API void swap(Buffer&);

    class Container : private IceUtil::noncopyable
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
	typedef int difference_type;
	typedef size_t size_type;

#ifdef ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION
	Container(size_type maxCapacity) :
	    _buf(_fixed),
	    _size(0),
	    _capacity(ICE_BUFFER_FIXED_SIZE),
	    _maxCapacity(maxCapacity)
	{
	}
#else
	Container(size_type maxCapacity) :
	    _buf(0),
	    _size(0),
	    _capacity(0),
	    _maxCapacity(maxCapacity)
	{
	}
#endif

	~Container()
	{
#ifdef ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION
	    if(_buf != _fixed)
	    {
		free(_buf);
	    }
#else
	    free(_buf);
#endif
	}

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

	ICE_API void swap(Container&);
	
	ICE_API void clear();

	void resize(size_type n) // Inlined for performance reasons.
        {
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

	ICE_API Container(const Container&);
	ICE_API void operator=(const Container&);
	ICE_API void reserve(size_type);

	pointer _buf;
	size_type _size;
	size_type _capacity;
	size_type _maxCapacity;
	int _shrinkCounter;

#ifdef ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION
	//
	// For small buffers, we stack-allocate the memory. Only when
	// a buffer size larger than _fixedSize is requested, we
	// allocate memory dynamically.
	//
	value_type _fixed[ICE_BUFFER_FIXED_SIZE];
#endif
    };

    Container b;
    Container::iterator i;
};

}

#endif
