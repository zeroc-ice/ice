// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#ifndef ICE_NO_LOCATOR

#include <IceE/IceE.h>
#include <IceE/Locator.h>
#include <TestI.h>

using namespace Test;

ServerManagerI::ServerManagerI(const IceE::ObjectAdapterPtr& adapter, const ServerLocatorRegistryPtr& registry) :
    _adapter(adapter), _registry(registry)
{
}

void
ServerManagerI::startServer(const IceE::Current&)
{
    int argc = 0;
    char** argv = 0;

    //
    // Simulate a server: create a new communicator and object
    // adapter. The object adapter is started on a system allocated
    // port. The configuration used here contains the IceE.Locator
    // configuration variable. The new object adapter will register
    // its endpoints with the locator and create references containing
    // the adapter id instead of the endpoints.
    //
    IceE::CommunicatorPtr serverCommunicator = IceE::initialize(argc, argv);
    _communicators.push_back(serverCommunicator);
    serverCommunicator->getProperties()->setProperty("TestAdapter.Endpoints", "default");
    serverCommunicator->getProperties()->setProperty("TestAdapter.AdapterId", "TestAdapter");
    IceE::ObjectAdapterPtr adapter = serverCommunicator->createObjectAdapter("TestAdapter");

    serverCommunicator->getProperties()->setProperty("TestAdapter2.Endpoints", "default");
    serverCommunicator->getProperties()->setProperty("TestAdapter2.AdapterId", "TestAdapter2");
    IceE::ObjectAdapterPtr adapter2 = serverCommunicator->createObjectAdapter("TestAdapter2");

    IceE::ObjectPrx locator = serverCommunicator->stringToProxy("locator:default -p 12345");
    adapter->setLocator(IceE::LocatorPrx::uncheckedCast(locator));
    adapter2->setLocator(IceE::LocatorPrx::uncheckedCast(locator));

    IceE::ObjectPtr object = new TestI(adapter, adapter2, _registry);
    _registry->addObject(adapter->add(object, IceE::stringToIdentity("test")));
    _registry->addObject(adapter->add(object, IceE::stringToIdentity("test2")));

    adapter->activate();
    adapter2->activate();
}

void
ServerManagerI::shutdown(const IceE::Current&)
{
    for(::std::vector<IceE::CommunicatorPtr>::const_iterator i = _communicators.begin(); i != _communicators.end(); ++i)
    {
	(*i)->destroy();
    }
    _adapter->getCommunicator()->shutdown();
}


TestI::TestI(const IceE::ObjectAdapterPtr& adapter, 
	     const IceE::ObjectAdapterPtr& adapter2, 
	     const ServerLocatorRegistryPtr& registry) :
    _adapter1(adapter), _adapter2(adapter2), _registry(registry)
{
    _registry->addObject(_adapter1->add(new HelloI(), IceE::stringToIdentity("hello")));
}

void
TestI::shutdown(const IceE::Current&)
{
    _adapter1->getCommunicator()->shutdown();
}

HelloPrx
TestI::getHello(const IceE::Current&)
{
    return HelloPrx::uncheckedCast(_adapter1->createProxy(IceE::stringToIdentity("hello")));
}

void
TestI::migrateHello(const IceE::Current&)
{
    const IceE::Identity id = IceE::stringToIdentity("hello");
    try
    {
	_registry->addObject(_adapter2->add(_adapter1->remove(id), id));
    }
    catch(IceE::NotRegisteredException&)
    {
	_registry->addObject(_adapter1->add(_adapter2->remove(id), id));
    }
}

void
HelloI::sayHello(const IceE::Current&)
{
}

#endif
