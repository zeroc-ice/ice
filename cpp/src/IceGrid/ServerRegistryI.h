// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_SERVER_REGISTRY_I_H
#define ICE_GRID_SERVER_REGISTRY_I_H

#include <IceGrid/Internal.h>
#include <IceGrid/StringObjectProxyDict.h>
#include <IceGrid/StringServerDescriptorDict.h>

namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ServerRegistryI : public ServerRegistry
{
public:

    ServerRegistryI(const Ice::CommunicatorPtr&, const std::string&, const TraceLevelsPtr&);

    virtual void add(const std::string&, const ServerPrx&, const ServerDescriptorPtr&, const ::Ice::Current&);
    virtual ServerPrx remove(const std::string&, const ::Ice::Current&);

    virtual ServerPrx findByName(const ::std::string&, const ::Ice::Current&);
    virtual ServerDescriptorPtr getDescriptor(const ::std::string&, const Ice::Current&);
    virtual Ice::StringSeq getAll(const ::Ice::Current&) const;
    virtual ServerDescriptorSeq getAllDescriptorsOnNode(const std::string&, const ::Ice::Current&) const;

private:

    static const std::string _dbName;
    static const std::string _dbDescriptorName;

    Freeze::ConnectionPtr _connectionCache;
    StringObjectProxyDict _dictCache;
    StringServerDescriptorDict _dictDescriptorCache;
    TraceLevelsPtr _traceLevels;
    const std::string _envName;
    const Ice::CommunicatorPtr _communicator;
};

}

#endif
