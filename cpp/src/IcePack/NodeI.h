// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_NODE_I_H
#define ICE_PACK_NODE_I_H

#include <IcePack/Internal.h>
#include <IcePack/Activator.h>

namespace IcePack
{

class NodeI : public Node
{
public:

    NodeI(const ActivatorPtr&, const std::string&, const ServerDeployerPrx&);

    virtual std::string getName(const Ice::Current& = Ice::Current()) const;
    virtual IcePack::ServerDeployerPrx getServerDeployer(const Ice::Current& = Ice::Current()) const;
    virtual void shutdown(const Ice::Current& = Ice::Current()) const;

private:

    ActivatorPtr _activator;
    std::string _name;
    ServerDeployerPrx _deployer;

};

};

#endif
