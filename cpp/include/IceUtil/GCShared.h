// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_UTIL_GC_SHARED_H
#define ICE_UTIL_GC_SHARED_H

#include <IceUtil/Config.h>
#include <IceUtil/GCRecMutex.h>
#include <set>

namespace IceUtil
{

class GC;
class GCShared;

typedef std::set<GCShared*> GCObjectSet;
extern ICE_UTIL_API GCObjectSet gcObjects; // Set of pointers to all existing classes with class data members.

typedef std::multiset<GCShared*> GCObjectMultiSet;

class ICE_UTIL_API GCShared : public noncopyable
{
public:

    GCShared() : _ref(0), _noDelete(false) {}
    virtual ~GCShared() {}
    virtual void __incRef(); // First derived class with class data members overrides this.
    virtual void __decRef(); // Ditto.
    int __getRef() const;
    void __setNoDelete(bool);
    void __decRefUnsafe() { --_ref; }
    virtual void __gcReachable(GCObjectMultiSet&) const = 0;
    virtual void __gcClear() = 0;

protected:

    static void __addObject(GCObjectMultiSet&, GCShared*);
    int _ref;
    bool _noDelete;

    friend class IceUtil::GC; // Allows IceUtil::GC to read value of _ref.
};

}

#endif
