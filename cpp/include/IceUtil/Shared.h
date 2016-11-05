// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_SHARED_H
#define ICE_UTIL_SHARED_H

#include <IceUtil/Config.h>
#include <IceUtil/Atomic.h>

//
// Base classes for reference counted types. The IceUtil::Handle
// template can be used for smart pointers to types derived from these
// bases.
//
// IceUtil::SimpleShared
// =====================
//
// A non thread-safe base class for reference-counted types.
//
// IceUtil::Shared
// ===============
//
// A thread-safe base class for reference-counted types.
//
namespace IceUtil
{

class ICE_API SimpleShared
{
public:

    SimpleShared();
    SimpleShared(const SimpleShared&);

    virtual ~SimpleShared();

    SimpleShared& operator=(const SimpleShared&)
    {
        return *this;
    }

    void iceIncRef()
    {
        assert(_ref >= 0);
        ++_ref;
    }

    void iceDecRef()
    {
        assert(_ref > 0);
        if(--_ref == 0)
        {
            if(!_noDelete)
            {
                delete this;
            }
        }
    }

    int iceGetRef() const
    {
        return _ref;
    }

    void iceSetNoDelete(bool b)
    {
        _noDelete = b;
    }

private:

    int _ref;
    bool _noDelete;
};

class ICE_API Shared
{
public:

    //
    // Flag constant used by the Shared class. Derived classes
    // such as GCObject define more flag constants.
    //
    static const unsigned char NoDelete;

    Shared();
    Shared(const Shared&);

    virtual ~Shared()
    {
    }

    Shared& operator=(const Shared&)
    {
        return *this;
    }

    virtual void iceIncRef();
    virtual void iceDecRef();
    virtual int iceGetRef() const;
    virtual void iceSetNoDelete(bool);

    void iceSetFlag(unsigned char flag)
    {
        _flags |= flag;
    }

    void iceClearFlag(unsigned char flag)
    {
        _flags &= ~flag;
    }

    bool iceHasFlag(unsigned char flag)
    {
        return (_flags & flag) > 0;
    }
    
protected:

    IceUtilInternal::Atomic _ref;
    unsigned char _flags;
};

}

#endif
