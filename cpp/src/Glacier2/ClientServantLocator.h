// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier/SessionManagerF.h>
#include <Glacier2/ClientBlobject.h>

namespace Glacier
{

class ClientServantLocator : public Ice::ServantLocator
{
public:

    ClientServantLocator(const Ice::ObjectAdapterPtr&,
			 const Ice::ObjectAdapterPtr&,
			 const Glacier::SessionManagerPrx&);
    
    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

private:

    IceInternal::RoutingTablePtr _routingTable;
    Ice::Identity _routerId;
    Ice::RouterPtr _router;
    Glacier::ClientBlobjectPtr _blobject;
};

}
