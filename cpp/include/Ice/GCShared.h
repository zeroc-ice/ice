// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GC_SHARED_H
#define ICE_GC_SHARED_H

#include <Ice/Config.h>
#include <Ice/GCCountMap.h>

namespace IceInternal
{

class GC;

class ICE_API GCShared
{
public:

    GCShared();
    GCShared(const GCShared&);
    virtual ~GCShared() {}

    GCShared& operator=(const GCShared&)
    {
        return *this;
    }

    virtual void __incRef(); // First derived class with class data members overrides this.
    virtual void __decRef(); // Ditto.
    virtual void __addObject(GCCountMap&) {} // Ditto.
    virtual bool __usesClasses() { return false; } // Ditto.

    virtual int __getRef() const;
    virtual void __setNoDelete(bool);

    virtual void __gcReachable(GCCountMap&) const = 0;
    virtual void __gcClear() = 0;

    int __getRefUnsafe() const
    {
        return _ref;
    }

    void __decRefUnsafe()
    {
        --_ref;
    }

protected:

    void __gcIncRef();
    void __gcDecRef();

    int _ref;
    bool _noDelete;

    friend class IceInternal::GC; // Allows IceInternal::GC to read value of _ref.
};

}

#endif
