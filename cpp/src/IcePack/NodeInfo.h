// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_NODE_INFO_H
#define ICE_PACK_NODE_INFO_H

#include <IcePack/Internal.h>
#include <Yellow/Yellow.h>

namespace IcePack
{

class ServerFactory;
typedef IceUtil::Handle<ServerFactory> ServerFactoryPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class NodeInfo : public ::IceUtil::Shared
{
public:
    
    NodeInfo(const Ice::CommunicatorPtr&, const ServerFactoryPtr&, const NodePtr&, const TraceLevelsPtr&);

    Ice::CommunicatorPtr getCommunicator() const;
    ServerFactoryPtr getServerFactory() const;
    TraceLevelsPtr getTraceLevels() const;
    NodePtr getNode() const;
    
    AdapterRegistryPrx getAdapterRegistry() const;
    ServerRegistryPrx getServerRegistry() const;

    Yellow::QueryPrx getYellowQuery() const;
    Yellow::AdminPrx getYellowAdmin() const;

private:
    
    Ice::CommunicatorPtr _communicator;
    ServerFactoryPtr _serverFactory;
    NodePtr _node;
    TraceLevelsPtr _traceLevels;
};

typedef IceUtil::Handle<NodeInfo> NodeInfoPtr;

}

#endif
