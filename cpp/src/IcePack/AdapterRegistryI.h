// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_ADAPTER_REGISTRY_I_H
#define ICE_PACK_ADAPTER_REGISTRY_I_H

#include <IcePack/Internal.h>
#include <IcePack/StringObjectProxyDict.h>

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class AdapterRegistryI : public AdapterRegistry
{
public:

    AdapterRegistryI(const Ice::CommunicatorPtr&, const std::string&, const TraceLevelsPtr&);

    virtual void add(const std::string&, const AdapterPrx&, const ::Ice::Current&);
    virtual AdapterPrx remove(const std::string&, const AdapterPrx&, const ::Ice::Current&);

    virtual AdapterPrx findById(const ::std::string&, const ::Ice::Current&);
    virtual Ice::StringSeq getAll(const ::Ice::Current&) const;

private:

    static const std::string _dbName;

    Freeze::ConnectionPtr _connectionCache;
    StringObjectProxyDict _dictCache;
    TraceLevelsPtr _traceLevels;
    const std::string _envName;
    const Ice::CommunicatorPtr _communicator;

};

}

#endif
