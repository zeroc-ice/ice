// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

class ICE_API Buffer : public IceUtil::noncopyable
{
public:

    Buffer() : i(b.begin()) { }
    virtual ~Buffer() { }

    void swap(Buffer& other)
    {
	b.swap(other.b);
	std::swap(i, other.i);
    }

    class Container : public IceUtil::noncopyable
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

	Container() :
	    _buf(0),
	    _size(0),
	    _capacity(0)
	{
	}

	~Container()
	{
	    free(_buf);
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

	void swap(Container& other)
	{
	    std::swap(_buf, other._buf);
	    std::swap(_size, other._size);
	    std::swap(_capacity, other._capacity);
	}
	
	void clear()
	{
	    free(_buf);
	    _buf = 0;
	    _size = 0;
	    _capacity = 0;
	}

	void resize(size_type n)
	{
	    _size = n;
	    if(_size > _capacity)
	    {
		_capacity = std::max(_size, 2 * _capacity);
		if(_buf)
		{
		    _buf = reinterpret_cast<pointer>(realloc(_buf, _capacity));
		}
		else
		{
		    _buf = reinterpret_cast<pointer>(malloc(_capacity));
		}
	    }
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

	//
	// Special operations.
	//

	void copyFromVector(const std::vector<value_type>& v)
	{
	    if(v.empty())
	    {
		clear();
	    }
	    else
	    {
		resize(v.size());
		memcpy(_buf, &v[0], v.size());
	    }
	}

	void copyToVector(std::vector<value_type>& v)
	{
	    if(empty())
	    {
		v.clear();
	    }
	    else
	    {
		std::vector<value_type>(_buf, _buf + _size).swap(v);
	    }
	}

    private:

	Container(const Container&);
	void operator=(const Container&);

	pointer _buf;
	size_type _size;
	size_type _capacity;
    };

    Container b;
    Container::iterator i;
};

}

#endif
