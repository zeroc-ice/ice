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

	typedef std::vector<Ice::Byte>::value_type value_type;
	typedef std::vector<Ice::Byte>::iterator iterator;
	typedef std::vector<Ice::Byte>::const_iterator const_iterator;
	typedef std::vector<Ice::Byte>::reference reference;
	typedef std::vector<Ice::Byte>::const_reference const_reference;
	typedef std::vector<Ice::Byte>::pointer pointer;
	typedef std::vector<Ice::Byte>::difference_type difference_type;
	typedef std::vector<Ice::Byte>::size_type size_type;

	Container() { }

	iterator begin() { return _buf.begin(); }

	const_iterator begin() const { return _buf.begin(); }

	iterator end() { return _buf.end(); }

	const_iterator end() const { return _buf.end(); }

	size_type size() const { return _buf.size(); }

	bool empty() const { return _buf.empty(); }

	void swap(Container& other) { return _buf.swap(other._buf); }

	void resize(size_type n) { _buf.resize(n); }

	void push_back(value_type v) { _buf.push_back(v); }

	reference operator[](size_type n) { return _buf[n]; }

	const_reference operator[](size_type n) const { return _buf[n]; }

	//
	// Special operations.
	//
	void copyFromVector(const std::vector<Ice::Byte>& v) { _buf = v; }
	void copyToVector(std::vector<Ice::Byte>& v) const { v = _buf; }
	
    private:

	Container(const Container&);
	void operator=(const Container&);

	std::vector<Ice::Byte> _buf;
    };

    Container b;
    Container::iterator i;
};

}

#endif
