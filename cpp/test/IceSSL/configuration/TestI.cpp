// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Thread.h>
#include <TestI.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;

ServerI::ServerI(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
ServerI::destroy()
{
    _communicator->destroy();
}

Test::ServerPrx
ServerFactoryI::createServer(const Test::Properties& props, const Current& current)
{
    PropertiesPtr properties = createProperties();
    for(Test::Properties::const_iterator p = props.begin(); p != props.end(); ++p)
    {
	properties->setProperty(p->first, p->second);
    }

    int argc = 0;
    char* argv[] = { "" };
    CommunicatorPtr communicator = initializeWithProperties(argc, argv, properties);
    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("ServerAdapter", "ssl");
    ServerIPtr server = new ServerI(communicator);
    ObjectPrx obj = adapter->addWithUUID(server);
    _servers[obj->ice_getIdentity()] = server;
    adapter->activate();

    return Test::ServerPrx::uncheckedCast(obj);;
}

void
ServerFactoryI::destroyServer(const Test::ServerPrx& srv, const Ice::Current&)
{
    map<Identity, ServerIPtr>::iterator p = _servers.find(srv->ice_getIdentity());
    if(p != _servers.end())
    {
	p->second->destroy();
	_servers.erase(p);
    }
}

void
ServerFactoryI::shutdown(const Ice::Current& current)
{
    test(_servers.empty());
    current.adapter->getCommunicator()->shutdown();
}
