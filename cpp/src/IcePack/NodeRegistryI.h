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

    NodeRegistryI(const Ice::CommunicatorPtr&, const std::string&, const std::string&,
		  const AdapterRegistryPtr&, const AdapterFactoryPtr&, const TraceLevelsPtr&);

    virtual void add(const std::string&, const NodePrx&, const ::Ice::Current&);
    virtual void remove(const std::string&, const ::Ice::Current& = Ice::Current());

    virtual NodePrx findByName(const ::std::string&, const ::Ice::Current&);
    virtual Ice::StringSeq getAll(const ::Ice::Current&) const;

private:

    StringObjectProxyDict _dict;
    AdapterRegistryPtr _adapterRegistry;
    AdapterFactoryPtr _adapterFactory;
    TraceLevelsPtr _traceLevels;
};

}

#endif
