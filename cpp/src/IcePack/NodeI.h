// **********************************************************************
//
// Copyright (c) 2002
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
