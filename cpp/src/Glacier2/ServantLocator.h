// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/RouterI.h>

namespace Glacier2
{

class ClientServantLocator : public Ice::ServantLocator
{
public:

    ClientServantLocator(const Ice::ObjectAdapterPtr&);
    
    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

private:

    const Ice::Identity _routerId;
    const Glacier2::SessionRouterIPtr _sessionRouter;
};

class ServerServantLocator : public Ice::ServantLocator
{
public:

    ServerServantLocator(const Glacier2::ServerBlobjectPtr&);
    
    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

private:

    const Glacier2::ServerBlobjectPtr _serverBlobject;
};

}
