// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_SERVER_MANAGER_I_H
#define ICE_PACK_SERVER_MANAGER_I_H

#include <IceUtil/Mutex.h>
#include <IcePack/ServerManager.h>
#include <IcePack/Activator.h>
#include <set>


namespace IcePack
{

class ServerI : public Server, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:
    ServerI(const ::Ice::ObjectAdapterPtr&, const ActivatorPrx&);
    virtual ~ServerI();
    
    virtual ServerDescription getServerDescription(const ::Ice::Current&);
    virtual bool start(const ::Ice::Current&);
    virtual void terminationCallback(const ::Ice::Current&);
    virtual ServerState getState(const ::Ice::Current&);

    void setState(ServerState);

private:

    ::Ice::ObjectAdapterPtr _adapter;
    ActivatorPrx _activator;
};


class ServerManagerI : public ServerManager, public IceUtil::Mutex
{
public:

    ServerManagerI(const Ice::ObjectAdapterPtr&, const AdapterManagerPrx&, const ActivatorPrx&);
    virtual ~ServerManagerI();

    virtual ServerPrx create(const ServerDescription&, const ::Ice::Current&);
    virtual ServerPrx findByName(const ::std::string&, const ::Ice::Current&);
    virtual void remove(const ::std::string&, const ::Ice::Current&);
    virtual ServerNames getAll(const ::Ice::Current&);

private:

    ::Ice::ObjectAdapterPtr _adapter;
    ::std::set< ::std::string> _serverNames;
    AdapterManagerPrx _adapterManager;
    ActivatorPrx _activator;
};

}

#endif
