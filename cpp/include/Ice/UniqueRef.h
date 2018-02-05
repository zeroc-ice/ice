// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UNIQUE_REF_H
#define ICE_UNIQUE_REF_H

#ifdef __APPLE__

#include <CoreFoundation/CoreFoundation.h>

namespace IceInternal
{

//
// UniqueRef helper class for CoreFoundation classes, comparable to std::unique_ptr
//
template<typename R>
class UniqueRef
{
public:

    explicit UniqueRef(R ref = 0) :
        _ref(ref)
    {
    }

    ~UniqueRef()
    {
        if(_ref != 0)
        {
            CFRelease(_ref);
        }
    }

    R release()
    {
        R r = _ref;
        _ref = 0;
        return r;
    }

    void reset(R ref = 0)
    {
        //
        // Support "self-reset" for CF objects. This is useful if CF allocation methods return
        // the same object with an increased reference count.
        //
        //assert(ref == 0 || ref != _ref);

        if(_ref != 0)
        {
            CFRelease(_ref);
        }
        _ref = ref;
    }

    void retain(R ref)
    {
        reset(ref ? (R)CFRetain(ref) : ref);
    }

    R& get()
    {
        return _ref;
    }

    R get() const
    {
        return _ref;
    }

    operator bool() const
    {
        return _ref != 0;
    }

    void swap(UniqueRef& a)
    {
        R tmp = a._ref;
        a._ref = _ref;
        _ref = tmp;
    }

private:

    UniqueRef(UniqueRef&);
    UniqueRef& operator=(UniqueRef&);

    R _ref;
};

}

#endif

#endif
