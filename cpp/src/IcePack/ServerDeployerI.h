// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
		     const ::IcePack::Targets&, const ::Ice::Current& = ::Ice::Current());

    virtual void remove(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

private:

    NodeInfoPtr _nodeInfo;

};

}

#endif
