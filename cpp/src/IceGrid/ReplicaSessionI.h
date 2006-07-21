// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_REPLICA_SESSION_H
#define ICEGRID_REPLICA_SESSION_H

#include <IceGrid/Internal.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ReplicaSessionI : public ReplicaSession, public IceUtil::Mutex
{
public:

    ReplicaSessionI(const DatabasePtr&, const std::string&, const InternalRegistryPrx&, const ReplicaInfo&);

    virtual void keepAlive(const Ice::Current&);
    virtual int getTimeout(const Ice::Current&) const;
    virtual void destroy(const Ice::Current&);
    
    virtual IceUtil::Time timestamp() const;

    const InternalRegistryPrx& getProxy() const { return _proxy; }
    const ReplicaInfo& getReplicaInfo() const { return _info; }

private:
    
    const DatabasePtr _database;
    const TraceLevelsPtr _traceLevels;
    const std::string _name;
    const InternalRegistryPrx _proxy;
    const ReplicaInfo _info;
    IceUtil::Time _timestamp;
    bool _destroy;
};
typedef IceUtil::Handle<ReplicaSessionI> ReplicaSessionIPtr;

};

#endif
