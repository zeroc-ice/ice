// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_SERVICE_DEPLOYER_H
#define ICE_PACK_SERVICE_DEPLOYER_H

#include <IceUtil/Shared.h>
#include <IcePack/ComponentDeployer.h>


namespace IcePack
{
class ServerDeployer;

class ServiceDeployer : public ComponentDeployer
{
public:

    enum ServiceKind
    {
	ServiceKindStandard,
	ServiceKindFreeze
    };

    ServiceDeployer(const Ice::CommunicatorPtr&, ServerDeployer&, const std::map<std::string, std::string>&);

    void parse(const std::string&);

    ServerDeployer& getServerDeployer() const;

    void setKind(ServiceKind);
    void setEntryPoint(const std::string&);
    void setDBEnv(const std::string&);

private:

    ServerDeployer& _serverDeployer;

    ServiceKind _kind;
};

}

#endif
