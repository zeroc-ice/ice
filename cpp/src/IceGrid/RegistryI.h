// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_REGISTRYI_H
#define ICE_GRID_REGISTRYI_H

#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>
#include <Glacier2/PermissionsVerifierF.h>
#include <IceStorm/Service.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ReapThread;
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;    

class WaitQueue;
typedef IceUtil::Handle<WaitQueue> WaitQueuePtr;    

class ClientSessionManagerI;
typedef IceUtil::Handle<ClientSessionManagerI> ClientSessionManagerIPtr;    

class AdminSessionManagerI;
typedef IceUtil::Handle<AdminSessionManagerI> AdminSessionManagerIPtr;    

class RegistryI : public Registry
{
public:

    RegistryI(const Ice::CommunicatorPtr&);
    ~RegistryI();

    bool start(bool);
    void stop();

    virtual SessionPrx createSession(const std::string&, const std::string&, const Ice::Current&);
    virtual AdminSessionPrx createAdminSession(const std::string&, const std::string&, const Ice::Current&);

    virtual void shutdown();
    
private:

    void addWellKnownObject(const Ice::ObjectPrx&, const std::string&);
    void setupThreadPool(const Ice::PropertiesPtr&, const std::string&, int, int = 0);
    Glacier2::PermissionsVerifierPrx getPermissionsVerifier(const Ice::ObjectAdapterPtr&, const Ice::LocatorPrx&,
							    const std::string&, const std::string&);

    Ice::CommunicatorPtr _communicator;
    DatabasePtr _database;
    TraceLevelsPtr _traceLevels;
    ReapThreadPtr _nodeReaper;
    ReapThreadPtr _clientReaper;
    WaitQueuePtr _waitQueue;
    ClientSessionManagerIPtr _clientSessionManager;
    Glacier2::PermissionsVerifierPrx _clientVerifier;
    AdminSessionManagerIPtr _adminSessionManager;
    Glacier2::PermissionsVerifierPrx _adminVerifier;

    IceStorm::ServicePtr _iceStorm;
};
typedef IceUtil::Handle<RegistryI> RegistryIPtr;

}

#endif
