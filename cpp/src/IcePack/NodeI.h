// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

}

#endif
