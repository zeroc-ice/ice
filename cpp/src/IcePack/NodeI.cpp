// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/NodeI.h>

using namespace std;
using namespace IcePack;

IcePack::NodeI::NodeI(const ActivatorPtr& activator, const string& name, const ServerDeployerPrx& deployer) :
    _activator(activator),
    _name(name),
    _deployer(deployer)
{
}

std::string
IcePack::NodeI::getName(const Ice::Current&) const
{
    return _name;
}

ServerDeployerPrx
IcePack::NodeI::getServerDeployer(const Ice::Current&) const
{
    return _deployer;
}

void
IcePack::NodeI::shutdown(const Ice::Current&) const
{
    return _activator->shutdown();
}

