// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    AdminI(const Ice::CommunicatorPtr&, const NodeRegistryPtr&, const ServerRegistryPtr&, const AdapterRegistryPtr&);
    virtual ~AdminI();

    virtual void addApplication(const std::string&, const Targets&, const Ice::Current& = Ice::Current());
    virtual void removeApplication(const std::string&, const Ice::Current& = Ice::Current());

    virtual void addServer(const std::string&, const std::string&, const std::string&, const std::string&, 
			   const std::string&, const Targets&, const Ice::Current& = Ice::Current());
    virtual void removeServer(const ::std::string&, const Ice::Current&);

    virtual ServerDescription getServerDescription(const ::std::string&, const Ice::Current&) const;
    virtual ServerState getServerState(const ::std::string&, const Ice::Current& = Ice::Current()) const;
    virtual Ice::Int getServerPid(const ::std::string&, const Ice::Current&) const;
    virtual bool startServer(const ::std::string&, const Ice::Current&);
    virtual void stopServer(const ::std::string&, const Ice::Current&);
    virtual Ice::StringSeq getAllServerNames(const Ice::Current&) const;
    virtual ServerActivation getServerActivation(const ::std::string&, const Ice::Current&) const;
    virtual void setServerActivation(const ::std::string&, ServerActivation, const Ice::Current&);

    virtual ::std::string getAdapterEndpoints(const ::std::string&, const ::Ice::Current&) const;
    virtual Ice::StringSeq getAllAdapterNames(const ::Ice::Current&) const;

    virtual bool pingNode(const std::string&, const Ice::Current&) const;
    virtual void shutdownNode(const std::string&, const Ice::Current&);
    virtual Ice::StringSeq getAllNodeNames(const ::Ice::Current&) const;

    virtual void shutdown(const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
    NodeRegistryPtr _nodeRegistry;
    ServerRegistryPtr _serverRegistry;
    AdapterRegistryPtr _adapterRegistry;
};

}

#endif
