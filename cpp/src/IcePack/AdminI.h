// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_ADMIN_I_H
#define ICE_PACK_ADMIN_I_H

#include <IcePack/Admin.h>
#include <IcePack/ServerManagerF.h>
#include <IcePack/AdapterManagerF.h>

namespace IcePack
{

class AdminI : public Admin, public IceUtil::Mutex
{
public:

    AdminI(const Ice::CommunicatorPtr&, const Ice::CommunicatorPtr&, const ServerManagerPrx&, 
	   const AdapterManagerPrx&);
    virtual ~AdminI();

    virtual void addApplication(const std::string&, const Targets&, const Ice::Current& = Ice::Current());
    virtual void removeApplication(const std::string&, const Ice::Current& = Ice::Current());

    virtual void addServer(const std::string&, const std::string&, const std::string&, const std::string&,
			   const Targets&, const Ice::Current& = Ice::Current());
    virtual ServerDescription getServerDescription(const ::std::string&, const Ice::Current&) const;
    virtual ServerState getServerState(const ::std::string&, const Ice::Current& = Ice::Current()) const;
    virtual Ice::Int getServerPid(const ::std::string&, const Ice::Current&) const;
    virtual bool startServer(const ::std::string&, const Ice::Current&);
    virtual void removeServer(const ::std::string&, const Ice::Current&);
    virtual ServerNames getAllServerNames(const Ice::Current&) const;

    virtual void addAdapterWithEndpoints(const ::std::string&, const ::std::string&, const ::Ice::Current&);
    virtual void removeAdapter(const ::std::string&, const ::Ice::Current&);
    virtual ::std::string getAdapterEndpoints(const ::std::string&, const ::Ice::Current&) const;
    virtual AdapterNames getAllAdapterNames(const ::Ice::Current&) const;

    virtual void shutdown(const Ice::Current&);

private:

    Ice::CommunicatorPtr _shutdownCommunicator;
    Ice::CommunicatorPtr _backendCommunicator;
    ServerManagerPrx _serverManager;
    AdapterManagerPrx _adapterManager;
};

}

#endif
