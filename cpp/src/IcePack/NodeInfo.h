// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_PACK_NODE_INFO_H
#define ICE_PACK_NODE_INFO_H

#include <IcePack/Internal.h>

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
    ObjectRegistryPrx getObjectRegistry() const;
    ServerRegistryPrx getServerRegistry() const;

private:
    
    Ice::CommunicatorPtr _communicator;
    ServerFactoryPtr _serverFactory;
    NodePtr _node;
    TraceLevelsPtr _traceLevels;
};

typedef IceUtil::Handle<NodeInfo> NodeInfoPtr;

}

#endif
