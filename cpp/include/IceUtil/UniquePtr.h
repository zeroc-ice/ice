// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_UNIQUE_PTR_H
#define ICE_UTIL_UNIQUE_PTR_H

#include <IceUtil/Config.h>

namespace IceUtil
{

#ifdef ICE_CPP11_MAPPING

template<typename T>
using UniquePtr = std::unique_ptr<T>;

#else

template<typename T>
class UniquePtr
{
public:

    explicit UniquePtr(T* ptr = 0) :
        _ptr(ptr)
    {
    }

    UniquePtr(UniquePtr& o) :
        _ptr(o.release())
    {
    }

    UniquePtr& operator=(UniquePtr& o)
    {
        reset(o.release());
        return *this;
    }

    ~UniquePtr()
    {
        if(_ptr != 0)
        {
            delete _ptr;
        }
    }

    T* release()
    {
        T* r = _ptr;
        _ptr = 0;
        return r;
    }

    void reset(T* ptr = 0)
    {
        assert(ptr == 0 || ptr != _ptr);

        if(_ptr != 0)
        {
            delete _ptr;
        }
        _ptr = ptr;
    }

    T& operator*() const
    {
        return *_ptr;
    }
    
    T* operator->() const
    {
        return _ptr;
    }


    T* get() const
    {
        return _ptr;
    }

    operator bool() const
    {
        return _ptr != 0;
    }

    void swap(UniquePtr& a)
    {
        T* tmp = a._ptr;
        a._ptr = _ptr;
        _ptr = tmp;
    }

private:

    T* _ptr;
};

#endif

} // End of namespace IceUtil

#endif
