// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef REPLICA_H
#define REPLICA_H

#include <Ice/Ice.h>
#include <IceStorm/Election.h>
#include <set>

namespace IceStormElection
{

struct GroupNodeInfo
{
    GroupNodeInfo(int i);
    GroupNodeInfo(int i, LogUpdate l, const Ice::ObjectPrx& o = Ice::ObjectPrx());
    bool operator<(const GroupNodeInfo& rhs) const;
    bool operator==(const GroupNodeInfo& rhs) const;
    //
    // COMPILER FIX: Clang using libc++ requires to define operator=
    //
#if defined(__clang__) && defined(_LIBCPP_VERSION)
    GroupNodeInfo& operator=(const GroupNodeInfo&);
#endif
    const int id;
    const LogUpdate llu;
    const Ice::ObjectPrx observer;
};

class Replica : public virtual IceUtil::Shared
{
public:

    virtual LogUpdate getLastLogUpdate() const = 0;
    virtual void sync(const Ice::ObjectPrx&) = 0;
    virtual void initMaster(const std::set<IceStormElection::GroupNodeInfo>&, const LogUpdate&) = 0;
    virtual Ice::ObjectPrx getObserver() const = 0;
    virtual Ice::ObjectPrx getSync() const = 0;
};
typedef IceUtil::Handle<Replica> ReplicaPtr;

}

#endif // RELICA_H
