// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Locator.h>
#include <TestI.h>

using namespace Test;

ServerManagerI::ServerManagerI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
ServerManagerI::startServer(const Ice::Current&)
{
    int argc = 0;
    char** argv = 0;

    //
    // Simulate a server: create a new communicator and object
    // adapter. The object adapter is started on a system allocated
    // port. The configuration used here contains the Ice.Locator
    // configuration variable. The new object adapter will register
    // its endpoints with the locator and create references containing
    // the adapter id instead of the endpoints.
    //
    Ice::CommunicatorPtr serverCommunicator = Ice::initialize(argc, argv);
    _communicators.push_back(serverCommunicator);
    serverCommunicator->getProperties()->setProperty("TestAdapter.Endpoints", "default");
    serverCommunicator->getProperties()->setProperty("TestAdapter.AdapterId", "TestAdapter");
    Ice::ObjectAdapterPtr adapter = serverCommunicator->createObjectAdapter("TestAdapter");

    Ice::ObjectPrx locator = serverCommunicator->stringToProxy("locator:default -p 12345");
    adapter->setLocator(Ice::LocatorPrx::uncheckedCast(locator));

    Ice::ObjectPtr object = new TestI(adapter);
    Ice::ObjectPrx proxy = adapter->add(object, Ice::stringToIdentity("test"));
    adapter->activate();
}

void
ServerManagerI::shutdown(const Ice::Current&)
{
    for(::std::vector<Ice::CommunicatorPtr>::const_iterator i = _communicators.begin(); i != _communicators.end(); ++i)
    {
	(*i)->destroy();
    }
    _adapter->getCommunicator()->shutdown();
}


TestI::TestI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
    Ice::ObjectPtr servant = new HelloI();
    _adapter->add(servant, Ice::stringToIdentity("hello")); 
}

void
TestI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
}

HelloPrx
TestI::getHello(const Ice::Current&)
{
    return HelloPrx::uncheckedCast(_adapter->createProxy(Ice::stringToIdentity("hello")));
}

void
HelloI::sayHello(const Ice::Current&)
{
}
