// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
		     const ::Ice::StringSeq&, const ::Ice::Current& = ::Ice::Current());

    virtual void remove(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

private:

    NodeInfoPtr _nodeInfo;

};

}

#endif
