// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_SCOPED_ARRAY_H
#define ICE_UTIL_SCOPED_ARRAY_H

#include <IceUtil/Config.h>

namespace IceUtil
{

template<typename T>
class ScopedArray : private noncopyable
{
public:

    explicit ScopedArray(T* ptr = 0) :
        _ptr(ptr)
    {
    }

    ~ScopedArray()
    {
        if(_ptr != 0)
        {
            delete[] _ptr;
        }
    }

    void reset(T* ptr = 0)
    {
        assert(ptr == 0 || ptr != _ptr);
        if(_ptr != 0)
        {
            delete[] _ptr;
        }
        _ptr = ptr;
    }

    T& operator[](size_t i) const
    {
        assert(_ptr != 0);
        assert(i >= 0);
        return _ptr[i];
    }

    T* get() const
    {
        return _ptr;
    }

    void swap(ScopedArray& a)
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
