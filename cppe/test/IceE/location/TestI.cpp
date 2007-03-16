// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#ifdef ICEE_HAS_LOCATOR

#include <IceE/IceE.h>
#include <IceE/Locator.h>
#include <TestCommon.h>
#include <TestI.h>

using namespace Test;

ServerManagerI::ServerManagerI(const Ice::ObjectAdapterPtr& adapter, const ServerLocatorRegistryPtr& registry,
			       const Ice::InitializationData& initData) :
    _adapter(adapter), _registry(registry), _initData(initData)
{

    

}

void
ServerManagerI::startServer(const Ice::Current&)
{
    for(::std::vector<Ice::CommunicatorPtr>::const_iterator i = _communicators.begin(); i != _communicators.end(); ++i)
    {
	(*i)->waitForShutdown();
	(*i)->destroy();
    }
    _communicators.clear();

    //
    // Simulate a server: create a new communicator and object
    // adapter. The object adapter is started on a system allocated
    // port. The configuration used here contains the Ice.Locator
    // configuration variable. The new object adapter will register
    // its endpoints with the locator and create references containing
    // the adapter id instead of the endpoints.
    //
    Ice::CommunicatorPtr serverCommunicator = Ice::initialize(_initData);
    _communicators.push_back(serverCommunicator); 

    Ice::ObjectAdapterPtr adapter = serverCommunicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = serverCommunicator->createObjectAdapter("TestAdapter2");

    // Note that this assumes the server is running port 12010.
    Ice::ObjectPrx locator = serverCommunicator->stringToProxy("locator:default -p 12010");

    adapter->setLocator(Ice::LocatorPrx::uncheckedCast(locator));
    adapter2->setLocator(Ice::LocatorPrx::uncheckedCast(locator));

    Ice::ObjectPtr object = new TestI(adapter, adapter2, _registry);
    _registry->addObject(adapter->add(object, serverCommunicator->stringToIdentity("test")));
    _registry->addObject(adapter->add(object, serverCommunicator->stringToIdentity("test2")));

    adapter->activate();
    adapter2->activate();
}

void
ServerManagerI::shutdown(const Ice::Current&)
{
    //
    // Clear the registry.
    //
    _registry->clear();

    //
    // Destroy each of the communicators.
    //
    for(::std::vector<Ice::CommunicatorPtr>::const_iterator i = _communicators.begin(); i != _communicators.end(); ++i)
    {
	(*i)->destroy();
    }
    _adapter->getCommunicator()->shutdown();
#ifdef _WIN32_WCE
    tprintf("The server has shutdown, close the window to terminate the server.");
#endif
}

TestI::TestI(const Ice::ObjectAdapterPtr& adapter, 
	     const Ice::ObjectAdapterPtr& adapter2, 
	     const ServerLocatorRegistryPtr& registry) :
    _adapter1(adapter), _adapter2(adapter2), _registry(registry)
{
    _registry->addObject(_adapter1->add(new HelloI(), _adapter1->getCommunicator()->stringToIdentity("hello")));
}

void
TestI::shutdown(const Ice::Current&)
{
    _adapter1->getCommunicator()->shutdown();
}

HelloPrx
TestI::getHello(const Ice::Current&)
{
    return HelloPrx::uncheckedCast(_adapter1->createIndirectProxy(
    				_adapter1->getCommunicator()->stringToIdentity("hello")));
}

HelloPrx
TestI::getReplicatedHello(const Ice::Current&)
{
    return HelloPrx::uncheckedCast(_adapter1->createProxy(_adapter1->getCommunicator()->stringToIdentity("hello")));
}

void
TestI::migrateHello(const Ice::Current&)
{
    const Ice::Identity id = _adapter1->getCommunicator()->stringToIdentity("hello");
    try
    {
	_registry->addObject(_adapter2->add(_adapter1->remove(id), id));
    }
    catch(Ice::NotRegisteredException&)
    {
	_registry->addObject(_adapter1->add(_adapter2->remove(id), id));
    }
}

void
HelloI::sayHello(const Ice::Current&)
{
}

#endif
