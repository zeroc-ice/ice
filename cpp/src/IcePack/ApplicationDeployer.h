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

class ApplicationDeployer : public ComponentDeployer
{
public:

    ApplicationDeployer(const Ice::CommunicatorPtr&, const AdminPtr&, const std::vector<std::string>&);

    void parse(const std::string&);

    void addServer(const std::string&, const std::string&, const std::string&, const std::string&);

private:
    
    AdminPtr _admin;

};

}

#endif
