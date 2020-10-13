//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_SHARED_H
#define ICE_UTIL_SHARED_H

#include <IceUtil/Config.h>
#include <atomic>

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

    void __incRef()
    {
        assert(_ref >= 0);
        ++_ref;
    }

    void __decRef()
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

    int __getRef() const
    {
        return _ref;
    }

    void __setNoDelete(bool b)
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
    // Flag constant used by the Shared class.
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

    virtual void __incRef();
    virtual void __decRef();
    virtual int __getRef() const;
    virtual void __setNoDelete(bool);

    void __setFlag(unsigned char flag)
    {
        _flags |= flag;
    }

    void __clearFlag(unsigned char flag)
    {
        _flags &= ~flag;
    }

    bool __hasFlag(unsigned char flag)
    {
        return (_flags & flag) > 0;
    }

protected:

    std::atomic<int> _ref;
    unsigned char _flags;
};

}

#endif
