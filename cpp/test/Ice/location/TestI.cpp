// **********************************************************************
//
// Copyright (c) 2003
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

#include <Ice/Ice.h>
#include <TestI.h>

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
    _serverCommunicator = Ice::initialize(argc, argv);
    _serverCommunicator->getProperties()->setProperty("TestAdapter.Endpoints", "default");
    _serverCommunicator->getProperties()->setProperty("TestAdapter.AdapterId", "TestAdapter");
    Ice::ObjectAdapterPtr adapter = _serverCommunicator->createObjectAdapter("TestAdapter");

    Ice::ObjectPrx locator = _serverCommunicator->stringToProxy("locator:default -p 12345");
    adapter->setLocator(Ice::LocatorPrx::uncheckedCast(locator));

    Ice::ObjectPtr object = new TestI(adapter);
    Ice::ObjectPrx proxy = adapter->add(object, Ice::stringToIdentity("test"));
    adapter->activate();
}

void
ServerManagerI::shutdown(const Ice::Current&)
{
    _serverCommunicator->shutdown();
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
