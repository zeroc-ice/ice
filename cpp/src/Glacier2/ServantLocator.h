// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/ClientBlobject.h>
#include <Glacier2/ServerBlobject.h>

namespace Glacier2
{

class ClientServantLocator : public Ice::ServantLocator, public IceUtil::Mutex
{
public:

    ClientServantLocator(const Ice::ObjectAdapterPtr&);
    
    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

private:

    const Ice::CommunicatorPtr _communicator;
    const Ice::PropertiesPtr _properties;
    const Ice::LoggerPtr _logger;
    const Ice::ObjectAdapterPtr _clientAdapter;
    const Ice::Identity _routerId;
    const std::string _serverEndpoints;
    const int _traceLevel;

    struct Client
    {
	Ice::RouterPtr router;
	Glacier2::ClientBlobjectPtr clientBlobject;
	Ice::ObjectAdapterPtr serverAdapter;
    };

    int _serverAdapterCount;

    std::map<Ice::TransportInfoPtr, Client> _clientMap;
    std::map<Ice::TransportInfoPtr, Client>::iterator _clientMapHint;
};

class ServerServantLocator : public Ice::ServantLocator
{
public:

    ServerServantLocator(const Glacier2::ServerBlobjectPtr&);
    
    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

private:

    Glacier2::ServerBlobjectPtr _serverBlobject;
};

}
