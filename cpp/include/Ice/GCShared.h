// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GC_SHARED_H
#define ICE_GC_SHARED_H

#include <IceUtil/Shared.h>
#include <Ice/Config.h>
#include <Ice/GCCountMap.h>

namespace IceInternal
{

class GC;

class ICE_API GCShared : public virtual IceUtil::Shared
{
public:

    virtual ~GCShared() {}

    GCShared& operator=(const GCShared&)
    {
        return *this;
    }

    virtual void __incRef();
    virtual void __decRef();
    virtual int __getRef() const;
    virtual void __setNoDelete(bool);

    virtual void __gcReachable(GCCountMap&) const = 0;
    virtual void __gcClear() = 0;

    int __getRefUnsafe() const
    {
        return _ref;
    }
};

}

#endif
