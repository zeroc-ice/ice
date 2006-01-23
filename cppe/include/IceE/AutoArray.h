// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_AUTO_ARRAY_H
#define ICEE_AUTO_ARRAY_H

namespace IceUtil
{

template<typename T>
class auto_array
{
public:

    auto_array(const T* ptr = 0) :
        _ptr(ptr)
    {
    }

    ~auto_array()
    {
        if(_ptr != 0)
	{
	    delete[] _ptr;
	}
    }

    void reset(const T* ptr = 0)
    {
        if(_ptr != 0)
	{
	    delete[] _ptr;
	}
	_ptr = ptr;
    }

    T& operator[](size_t i) const
    {
        return _ptr[i];
    }

    T* get() const
    {
        return _ptr;
    }

    void swap(auto_array& a)
    {
        T* tmp = a._ptr;
	a._ptr = _ptr;
	_ptr = tmp;
    }

private:

    const T* _ptr;
};

}; // End of namespace IceUtil

#endif
