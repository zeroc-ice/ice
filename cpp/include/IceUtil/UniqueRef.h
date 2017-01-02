// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_UNIQUE_REF_H
#define ICE_UTIL_UNIQUE_REF_H

#include <IceUtil/Config.h>

#ifdef __APPLE__

#include <CoreFoundation/CoreFoundation.h>

namespace IceUtil
{
template<typename T>
class UniqueRef
{
public:

    explicit UniqueRef(CFTypeRef ptr = 0) : _ptr((T)ptr)
    {
    }

    ~UniqueRef()
    {
        if(_ptr != 0)
        {
            CFRelease(_ptr);
        }
    }

    T release()
    {
        T r = _ptr;
        _ptr = 0;
        return r;
    }

    void reset(CFTypeRef ptr = 0)
    {
        if(_ptr == ptr)
        {
            return;
        }
        if(_ptr != 0)
        {
            CFRelease(_ptr);
        }
        _ptr = (T)ptr;
    }

    void retain(CFTypeRef ptr)
    {
        reset(ptr ? CFRetain(ptr) : ptr);
    }

    T get() const
    {
        return _ptr;
    }

    operator bool() const
    {
        return _ptr != 0;
    }

private:

    T _ptr;
};

}
#endif

#endif

