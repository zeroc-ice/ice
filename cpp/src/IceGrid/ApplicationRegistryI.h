// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_APPLICATION_REGISTRY_I_H
#define ICE_GRID_APPLICATION_REGISTRY_I_H

#include <IceGrid/Internal.h>
#include <IceGrid/StringStringSeqDict.h>

namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ApplicationRegistryI : public ApplicationRegistry
{
public:

    ApplicationRegistryI(const Ice::CommunicatorPtr&, const ServerRegistryPtr&, const std::string&,
			 const TraceLevelsPtr&);

    virtual void add(const std::string&, const ::Ice::Current&);
    virtual void remove(const std::string&, const ::Ice::Current&);

    virtual void registerServer(const std::string&, const std::string&, const Ice::Current&);
    virtual void unregisterServer(const std::string&, const std::string&, const Ice::Current&);

    virtual ApplicationDescriptorPtr getDescriptor(const ::std::string&, const Ice::Current&);
    virtual Ice::StringSeq getAll(const ::Ice::Current&) const;

private:
    
    static const std::string _dbName;

    ServerRegistryPtr _serverRegistry;
    Freeze::ConnectionPtr _connectionCache;
    StringStringSeqDict _dictCache;
    TraceLevelsPtr _traceLevels;
    const std::string _envName;
    const Ice::CommunicatorPtr _communicator;
};

}

#endif
