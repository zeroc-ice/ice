// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_ADMIN_I_H
#define ICE_GRID_ADMIN_I_H

#include <IceGrid/Internal.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class AdminI : public Admin, public IceUtil::Mutex
{
public:

    AdminI(const Ice::CommunicatorPtr&, const DatabasePtr&, const RegistryPtr&);
    virtual ~AdminI();

    virtual void addApplication(const ApplicationDescriptorPtr&, const Ice::Current&);
    virtual void updateApplication(const ApplicationDescriptorPtr&, const Ice::Current&);
    virtual void removeApplication(const std::string&, const Ice::Current&);
    virtual ApplicationDescriptorPtr getApplicationDescriptor(const ::std::string&, const Ice::Current&) const;
    virtual Ice::StringSeq getAllApplicationNames(const Ice::Current&) const;

    virtual void addServer(const ServerDescriptorPtr&, const Ice::Current&);
    virtual void updateServer(const ServerDescriptorPtr&, const Ice::Current&);
    virtual void removeServer(const ::std::string&, const Ice::Current&);
    virtual ServerDescriptorPtr getServerDescriptor(const ::std::string&, const Ice::Current&) const;

    virtual ServerState getServerState(const ::std::string&, const Ice::Current&) const;
    virtual Ice::Int getServerPid(const ::std::string&, const Ice::Current&) const;
    virtual bool startServer(const ::std::string&, const Ice::Current&);
    virtual void stopServer(const ::std::string&, const Ice::Current&);
    virtual void sendSignal(const ::std::string&, const ::std::string&, const Ice::Current&);
    virtual void writeMessage(const ::std::string&, const ::std::string&, Ice::Int, const Ice::Current&);
    virtual Ice::StringSeq getAllServerNames(const Ice::Current&) const;
    virtual ServerActivation getServerActivation(const ::std::string&, const Ice::Current&) const;
    virtual void setServerActivation(const ::std::string&, ServerActivation, const Ice::Current&);

    virtual ::std::string getAdapterEndpoints(const ::std::string&, const ::Ice::Current&) const;
    virtual Ice::StringSeq getAllAdapterIds(const ::Ice::Current&) const;

    virtual void addObject(const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void updateObject(const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void addObjectWithType(const ::Ice::ObjectPrx&, const ::std::string&, const ::Ice::Current&);
    virtual void removeObject(const ::Ice::Identity&, const ::Ice::Current&);
    virtual ObjectDescriptor getObjectDescriptor(const Ice::Identity&, const ::Ice::Current&) const;
    virtual ObjectDescriptorSeq getAllObjectDescriptors(const std::string&, const ::Ice::Current&) const;

    virtual bool pingNode(const std::string&, const Ice::Current&) const;
    virtual void shutdownNode(const std::string&, const Ice::Current&);
    virtual void removeNode(const std::string&, const Ice::Current&);
    virtual std::string getNodeHostname(const std::string&, const Ice::Current&) const;
    virtual Ice::StringSeq getAllNodeNames(const ::Ice::Current&) const;

    virtual void shutdown(const Ice::Current&);

    virtual Ice::SliceChecksumDict getSliceChecksums(const Ice::Current&) const;

private:

    Ice::CommunicatorPtr _communicator;
    DatabasePtr _database;
    RegistryPtr _registry;
};

}

#endif
