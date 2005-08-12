// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BUFFER_H
#define ICE_BUFFER_H

#include <IceE/Config.h>

//#define ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION

namespace IceInternal
{

class ICE_API Buffer : private IceUtil::noncopyable
{
public:

    Buffer() : i(b.begin()) { }
    virtual ~Buffer() { }

    void swap(Buffer&);

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
	typedef int difference_type;
	typedef size_t size_type;

#ifdef ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION
	Container() :
	    _buf(_fixed),
	    _size(0),
	    _capacity(_fixedSize)
	{
	}
#else
	Container() :
	    _buf(0),
	    _size(0),
	    _capacity(0)
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

	void swap(Container&);
	
	void clear()
	{
#ifdef ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION
	    if(_buf != _fixed)
	    {
		free(_buf);
		_buf = _fixed;
	    }
	    _size = 0;
	    _capacity = _fixedSize;
#else
	    free(_buf);
	    _buf = 0;
	    _size = 0;
	    _capacity = 0;
#endif
	}

	void resize(size_type);

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

	pointer _buf;
	size_type _size;
	size_type _capacity;

#ifdef ICE_SMALL_MESSAGE_BUFFER_OPTIMIZATION
	//
	// For small buffers, we stack-allocate the memory. Only when
	// a buffer size larger than _fixedSize is requested, we
	// allocate memory dynamically.
	//
	static const size_type _fixedSize = 64;
	value_type _fixed[_fixedSize];
#endif
    };

    Container b;
    Container::iterator i;
};

}

#endif
