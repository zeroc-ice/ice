// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    _activator->shutdown();
}

