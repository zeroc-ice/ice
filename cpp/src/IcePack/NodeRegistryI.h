// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_NODE_REGISTRY_I_H
#define ICE_PACK_NODE_REGISTRY_I_H

#include <IcePack/Internal.h>
#include <IcePack/StringObjectProxyDict.h>

namespace IcePack
{

class AdapterFactory;
typedef IceUtil::Handle<AdapterFactory> AdapterFactoryPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class NodeRegistryI : public NodeRegistry, public IceUtil::Mutex
{
public:

    NodeRegistryI(const Ice::CommunicatorPtr&, const std::string&, const AdapterRegistryPtr&, 
		  const AdapterFactoryPtr&, const TraceLevelsPtr&);

    virtual void add(const std::string&, const NodePrx&, const ::Ice::Current&);
    virtual void remove(const std::string&, const ::Ice::Current& = Ice::Current());

    virtual NodePrx findByName(const ::std::string&, const ::Ice::Current&);
    virtual Ice::StringSeq getAll(const ::Ice::Current&) const;

private:

    static const std::string _dbName;

    Freeze::ConnectionPtr _connectionCache;
    StringObjectProxyDict _dictCache;
    AdapterRegistryPtr _adapterRegistry;
    AdapterFactoryPtr _adapterFactory;
    TraceLevelsPtr _traceLevels;
    const std::string _envName;
    const Ice::CommunicatorPtr _communicator;
};

}

#endif
