// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************


#ifndef ICE_PACK_SERVER_DEPLOYER_H
#define ICE_PACK_SERVER_DEPLOYER_H

#include <IceUtil/Shared.h>
#include <IcePack/ComponentDeployer.h>
#include <IcePack/ServerManagerF.h>
#include <IcePack/AdapterManagerF.h>

namespace IcePack
{

class ServerDeployer : public ComponentDeployer
{
public:
    enum ServerKind
    {
	CppIceBox,
	JavaIceBox,
	CppServer,
	JavaServer
    };

    ServerDeployer(const Ice::CommunicatorPtr&, const ServerPtr&, const ServerPrx&);

    void setAdapterManager(const AdapterManagerPrx&);

    virtual void parse();

    void setClassName(const std::string&);
    void setWorkingDirectory(const std::string&);
    void addAdapter(const std::string&);
    void addService(const std::string&, const std::string&);
    void addOption(const std::string&);
    void addJavaOption(const std::string&);
    void setKind(ServerKind);

private:

    AdapterManagerPrx _adapterManager;

    ServerKind _kind;

    ServerPtr _server;
    ServerPrx _serverProxy;

    std::string _className;
    std::vector<std::string> _javaOptions;
};

}

#endif
