// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    GCShared();
    virtual ~GCShared();

    virtual void __incRef(); // First derived class with class data members overrides this.
    virtual void __decRef(); // Ditto.
    int __getRef() const;
    void __setNoDelete(bool);

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
