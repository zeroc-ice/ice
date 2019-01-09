// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Locator.h>
#include <TestI.h>
#include <TestHelper.h>

using namespace Test;

ServerManagerI::ServerManagerI(const ServerLocatorRegistryPtr& registry,
                               const Ice::InitializationData& initData) :
    _registry(registry),
    _initData(initData),
    _nextPort(1)
{
    _initData.properties->setProperty("TestAdapter.AdapterId", "TestAdapter");
    _initData.properties->setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter");
    _initData.properties->setProperty("TestAdapter2.AdapterId", "TestAdapter2");
    _initData.properties->setProperty("Ice.PrintAdapterReady", "0");
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

    //
    // Use fixed port to ensure that OA re-activation doesn't re-use previous port from
    // another OA (e.g.: TestAdapter2 is re-activated using port of TestAdapter).
    //
    int nRetry = 10;
    while(--nRetry > 0)
    {
        Ice::ObjectAdapterPtr adapter;
        Ice::ObjectAdapterPtr adapter2;
        try
        {
            Ice::PropertiesPtr props = _initData.properties;
            serverCommunicator->getProperties()->setProperty("TestAdapter.Endpoints",
                                                             TestHelper::getTestEndpoint(props, _nextPort++));
            serverCommunicator->getProperties()->setProperty("TestAdapter2.Endpoints",
                                                             TestHelper::getTestEndpoint(props, _nextPort++));

            adapter = serverCommunicator->createObjectAdapter("TestAdapter");
            adapter2 = serverCommunicator->createObjectAdapter("TestAdapter2");

            Ice::ObjectPrxPtr locator =
                serverCommunicator->stringToProxy("locator:" + TestHelper::getTestEndpoint(props));
            adapter->setLocator(ICE_UNCHECKED_CAST(Ice::LocatorPrx, locator));
            adapter2->setLocator(ICE_UNCHECKED_CAST(Ice::LocatorPrx, locator));

            Ice::ObjectPtr object = ICE_MAKE_SHARED(TestI, adapter, adapter2, _registry);
            _registry->addObject(adapter->add(object, Ice::stringToIdentity("test")));
            _registry->addObject(adapter->add(object, Ice::stringToIdentity("test2")));
            adapter->add(object, Ice::stringToIdentity("test3"));

            adapter->activate();
            adapter2->activate();
            break;
        }
        catch(const Ice::SocketException&)
        {
            if(nRetry == 0)
            {
                throw;
            }

            // Retry, if OA creation fails with EADDRINUSE (this can occur when running with JS web
            // browser clients if the driver uses ports in the same range as this test, ICE-8148)
            if(adapter)
            {
                adapter->destroy();
            }
            if(adapter2)
            {
                adapter2->destroy();
            }
        }
    }
}

void
ServerManagerI::shutdown(const Ice::Current& current)
{
    for(::std::vector<Ice::CommunicatorPtr>::const_iterator i = _communicators.begin(); i != _communicators.end(); ++i)
    {
        (*i)->destroy();
    }
    current.adapter->getCommunicator()->shutdown();
}

TestI::TestI(const Ice::ObjectAdapterPtr& adapter,
             const Ice::ObjectAdapterPtr& adapter2,
             const ServerLocatorRegistryPtr& registry) :
    _adapter1(adapter), _adapter2(adapter2), _registry(registry)
{
    _registry->addObject(_adapter1->add(ICE_MAKE_SHARED(HelloI), Ice::stringToIdentity("hello")));
}

void
TestI::shutdown(const Ice::Current&)
{
    _adapter1->getCommunicator()->shutdown();
}

HelloPrxPtr
TestI::getHello(const Ice::Current&)
{
    return ICE_UNCHECKED_CAST(HelloPrx, _adapter1->createIndirectProxy(
                                            Ice::stringToIdentity("hello")));
}

HelloPrxPtr
TestI::getReplicatedHello(const Ice::Current&)
{
    return ICE_UNCHECKED_CAST(HelloPrx, _adapter1->createProxy(Ice::stringToIdentity("hello")));
}

void
TestI::migrateHello(const Ice::Current&)
{
    const Ice::Identity id = Ice::stringToIdentity("hello");
    try
    {
        _registry->addObject(_adapter2->add(_adapter1->remove(id), id));
    }
    catch(const Ice::NotRegisteredException&)
    {
        _registry->addObject(_adapter1->add(_adapter2->remove(id), id));
    }
}

void
HelloI::sayHello(const Ice::Current&)
{
}
