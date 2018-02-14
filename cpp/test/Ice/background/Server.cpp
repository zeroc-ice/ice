// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <Configuration.h>
#include <PluginI.h>

#include <Ice/Locator.h>
#include <Ice/Router.h>

using namespace std;

class LocatorI : public Ice::Locator
{
public:

    virtual void
    findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr& response, const string&,
                          const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
        response->ice_response(current.adapter->createDirectProxy(communicator->stringToIdentity("dummy")));
    }

    virtual void
    findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr& response, const Ice::Identity& id,
                         const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
        response->ice_response(current.adapter->createDirectProxy(id));
    }

    virtual Ice::LocatorRegistryPrx
    getRegistry(const Ice::Current&) const
    {
        return 0;
    }

    LocatorI(const BackgroundControllerIPtr& controller) : _controller(controller)
    {
    }

private:

    BackgroundControllerIPtr _controller;
};

class RouterI : public Ice::Router
{
public:

    virtual Ice::ObjectPrx
    getClientProxy(const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        return 0;
    }

    virtual Ice::ObjectPrx
    getServerProxy(const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        return 0;
    }

    virtual Ice::ObjectProxySeq
    addProxies(const Ice::ObjectProxySeq&, const Ice::Current&)
    {
        return Ice::ObjectProxySeq();
    }

    RouterI(const BackgroundControllerIPtr& controller)
    {
        _controller = controller;
    }

private:

    BackgroundControllerIPtr _controller;
};


int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12010");
    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", "tcp -p 12011");
    communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("ControllerAdapter");

    PluginI* plugin = dynamic_cast<PluginI*>(communicator->getPluginManager()->getPlugin("Test").get());
    assert(plugin);
    ConfigurationPtr configuration = plugin->getConfiguration();
    BackgroundControllerIPtr backgroundController = new BackgroundControllerI(adapter, configuration);

    adapter->add(new BackgroundI(backgroundController), communicator->stringToIdentity("background"));
    adapter->add(new LocatorI(backgroundController), communicator->stringToIdentity("locator"));
    adapter->add(new RouterI(backgroundController), communicator->stringToIdentity("router"));
    adapter->activate();

    adapter2->add(backgroundController, communicator->stringToIdentity("backgroundController"));
    adapter2->activate();

    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties->setProperty("Ice.Warn.Connections", "0");

        initData.properties->setProperty("Ice.MessageSizeMax", "50000");

        // This test relies on filling the TCP send/recv buffer, so
        // we rely on a fixed value for these buffers.
        initData.properties->setProperty("Ice.TCP.RcvSize", "50000");

        //
        // Setup the test transport plug-in.
        //
        initData.properties->setProperty("Ice.Plugin.Test", "TestTransport:createTestTransport");
        string defaultProtocol = initData.properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");
        initData.properties->setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);

        communicator = Ice::initialize(argc, argv, initData);
        status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
