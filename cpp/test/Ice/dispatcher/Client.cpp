// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>
#include <Dispatcher.h>

using namespace std;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    //
    // Limit the send buffer size, this test relies on the socket
    // send() blocking after sending a given amount of data.
    //
    initData.properties->setProperty("Ice.TCP.SndSize", "50000");

#ifdef ICE_CPP11_MAPPING
    IceUtil::Handle<Dispatcher> dispatcher = new Dispatcher;
    initData.dispatcher = [=](function<void()> call, const shared_ptr<Ice::Connection>& conn)
        {
            dispatcher->dispatch(make_shared<DispatcherCall>(call), conn);
        };
#else
    initData.dispatcher = new Dispatcher();
#endif
    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);

    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
