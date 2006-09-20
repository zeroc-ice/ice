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

class WellKnownObjectsManager;
typedef IceUtil::Handle<WellKnownObjectsManager> WellKnownObjectsManagerPtr;

class ReapThread;
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;    

class RegistryI;
typedef IceUtil::Handle<RegistryI> RegistryIPtr;

class ReplicaSessionManager;

class InternalRegistryI : public InternalRegistry
{
public:

    InternalRegistryI(const RegistryIPtr&, const DatabasePtr&, const ReapThreadPtr&, 
		      const WellKnownObjectsManagerPtr&, ReplicaSessionManager&);
    virtual ~InternalRegistryI();

    virtual NodeSessionPrx registerNode(const std::string&, const NodePrx&, const NodeInfo&, const Ice::Current&);
    virtual ReplicaSessionPrx registerReplica(const std::string&, const RegistryInfo&, const InternalRegistryPrx&,
					      const Ice::Current&);

    virtual void registerWithReplica(const InternalRegistryPrx&, const Ice::Current&);

    virtual NodePrxSeq getNodes(const Ice::Current&) const;
    virtual InternalRegistryPrxSeq getReplicas(const Ice::Current&) const;

    virtual void shutdown(const Ice::Current&) const;

private:    

    const RegistryIPtr _registry;
    const DatabasePtr _database;
    const ReapThreadPtr _reaper;
    const WellKnownObjectsManagerPtr _wellKnownObjects;
    ReplicaSessionManager& _session;
    int _nodeSessionTimeout;
    int _replicaSessionTimeout;
};
    
};

#endif
