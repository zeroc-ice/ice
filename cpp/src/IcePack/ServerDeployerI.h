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

#ifndef ICE_PACK_SERVER_DEPLOYER_I_H
#define ICE_PACK_SERVER_DEPLOYER_I_H

#include <IcePack/Internal.h>
#include <IcePack/NodeInfo.h>

namespace IcePack
{


class ServerDeployerI : public ServerDeployer, public IceUtil::Mutex
{
public:

    ServerDeployerI(const NodeInfoPtr&);
    virtual ~ServerDeployerI();

    virtual void add(const ::std::string&, const ::std::string&, const ::std::string&, const ::std::string&,
		     const ::IcePack::ServerTargets&, const ::Ice::Current& = ::Ice::Current());

    virtual void remove(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

private:

    NodeInfoPtr _nodeInfo;

};

}

#endif
