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

#ifndef ICE_PACK_ADMIN_I_H
#define ICE_PACK_ADMIN_I_H

#include <IcePack/Internal.h>

namespace IcePack
{

class AdminI : public Admin, public IceUtil::Mutex
{
public:

    AdminI(const Ice::CommunicatorPtr&, const NodeRegistryPtr&, const ServerRegistryPtr&, const AdapterRegistryPtr&,
	   const ObjectRegistryPtr&);
    virtual ~AdminI();

    virtual void addApplication(const std::string&, const Ice::StringSeq&, const Ice::Current& = Ice::Current());
    virtual void removeApplication(const std::string&, const Ice::Current& = Ice::Current());

    virtual void addServer(const std::string&, const std::string&, const std::string&, const std::string&, 
			   const std::string&, const Ice::StringSeq&, const Ice::Current& = Ice::Current());
    virtual void removeServer(const ::std::string&, const Ice::Current&);

    virtual ServerDescription getServerDescription(const ::std::string&, const Ice::Current&) const;
    virtual ServerState getServerState(const ::std::string&, const Ice::Current& = Ice::Current()) const;
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

    virtual void addObject(const ::Ice::ObjectPrx&, const ::Ice::Current&) const;
    virtual void addObjectWithType(const ::Ice::ObjectPrx&, const ::std::string&, const ::Ice::Current&) const;
    virtual void removeObject(const ::Ice::ObjectPrx&, const ::Ice::Current&) const;

    virtual bool pingNode(const std::string&, const Ice::Current&) const;
    virtual void shutdownNode(const std::string&, const Ice::Current&);
    virtual Ice::StringSeq getAllNodeNames(const ::Ice::Current&) const;

    virtual void shutdown(const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
    NodeRegistryPtr _nodeRegistry;
    ServerRegistryPtr _serverRegistry;
    AdapterRegistryPtr _adapterRegistry;
    ObjectRegistryPtr _objectRegistry;
};

}

#endif
