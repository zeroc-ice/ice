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
#include <IcePack/Admin.h>
#include <IcePack/ServerManagerF.h>
#include <IcePack/AdapterManagerF.h>

namespace IcePack
{

class ServerDeployer : public ComponentDeployer
{
public:

    enum ServerKind
    {
	ServerKindCppIceBox,
	ServerKindJavaIceBox,
	ServerKindCppServer,
	ServerKindJavaServer
    };

    ServerDeployer(const Ice::CommunicatorPtr&, const std::string&, const std::string&, const std::string&,
		   const std::vector<std::string>&);

    void setServerManager(const ServerManagerPrx&);
    void setAdapterManager(const AdapterManagerPrx&);

    void parse(const std::string&);

    virtual void deploy();
    virtual void undeploy();

    const ServerDescription& getServerDescription() const;

    void setClassName(const std::string&);
    void setWorkingDirectory(const std::string&);
    void addAdapter(const std::string&, const std::string&);
    void addService(const std::string&, const std::string&);
    void addOption(const std::string&);
    void addJavaOption(const std::string&);
    void setKind(ServerKind);

private:

    ServerManagerPrx _serverManager;
    AdapterManagerPrx _adapterManager;

    ServerKind _kind;
    ServerDescription _description;
    std::string _className;
    std::string _libraryPath;
    std::vector<std::string> _javaOptions;
};

}

#endif
