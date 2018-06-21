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
#include <TestI.h>
#include <WstringI.h>
#include <StringConverterI.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    setProcessStringConverter(ICE_MAKE_SHARED(Test::StringConverterI));
    setProcessWstringConverter(ICE_MAKE_SHARED(Test::WstringConverterI));
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("TEST"));
    adapter->add(ICE_MAKE_SHARED(Test1::WstringClassI), Ice::stringToIdentity("WSTRING1"));
    adapter->add(ICE_MAKE_SHARED(Test2::WstringClassI), Ice::stringToIdentity("WSTRING2"));

    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
