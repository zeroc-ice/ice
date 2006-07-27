// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_INTERNALREGISTRYI_H
#define ICE_GRID_INTERNALREGISTRYI_H

#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class ReapThread;
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;    

class ReplicaSessionManager;

class InternalRegistryI : public InternalRegistry
{
public:

    InternalRegistryI(const DatabasePtr&, const ReapThreadPtr&, ReplicaSessionManager&);
    virtual ~InternalRegistryI();

    virtual NodeSessionPrx registerNode(const std::string&, const NodePrx&, const NodeInfo&, const Ice::Current&);
    virtual ReplicaSessionPrx registerReplica(const std::string&, const InternalRegistryPrx&, 
					      const RegistryObserverPrx&, const Ice::Current&);

    virtual void registerWithReplica(const InternalRegistryPrx&, const Ice::Current&);

    virtual NodePrxSeq getNodes(const Ice::Current&) const;
    virtual InternalRegistryPrxSeq getReplicas(const Ice::Current&) const;

private:    

    const DatabasePtr _database;
    const ReapThreadPtr _reaper;
    ReplicaSessionManager& _session;
};
    
};

#endif
