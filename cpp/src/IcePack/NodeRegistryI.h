// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    NodeRegistryI(const Freeze::DBPtr&, const AdapterRegistryPtr&, const AdapterFactoryPtr&, const TraceLevelsPtr&);

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
