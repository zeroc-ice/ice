// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>
#include <Dispatcher.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    //
    // Limit the recv buffer size, this test relies on the socket
    // send() blocking after sending a given amount of data.
    //
    initData.properties->setProperty("Ice.TCP.RcvSize", "50000");

#ifdef ICE_CPP11_MAPPING
    IceUtil::Handle<Dispatcher> dispatcher = new Dispatcher;
    initData.dispatcher = [=](function<void()> call, const shared_ptr<Ice::Connection>& conn)
        {
            dispatcher->dispatch(make_shared<DispatcherCall>(call), conn);
        };
#else
    initData.dispatcher = new Dispatcher();
#endif
    // The communicator must be destroyed before the dispatcher is terminated.
    {
        Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);

        communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
        communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
        communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
        Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("ControllerAdapter");

        TestIntfControllerIPtr testController = ICE_MAKE_SHARED(TestIntfControllerI, adapter);

        adapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("test"));
        adapter->activate();

        adapter2->add(testController, Ice::stringToIdentity("testController"));
        adapter2->activate();

        serverReady();

        communicator->waitForShutdown();
    }
    Dispatcher::terminate();
}

DEFINE_TEST(Server)
