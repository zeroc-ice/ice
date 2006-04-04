// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_AUTO_ARRAY_H
#define ICE_UTIL_AUTO_ARRAY_H

namespace IceUtil
{

template<typename T>
class auto_array
{
public:

    auto_array(T* ptr = 0) :
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

    void reset(T* ptr = 0)
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

    T* _ptr;
};

} // End of namespace IceUtil

#endif
