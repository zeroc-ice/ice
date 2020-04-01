//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    setProcessStringConverter(std::make_shared<Test::StringConverterI>());
    setProcessWstringConverter(std::make_shared<Test::WstringConverterI>());
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("TEST"));
    adapter->add(std::make_shared<Test1::WstringClassI>(), Ice::stringToIdentity("WSTRING1"));
    adapter->add(std::make_shared<Test2::WstringClassI>(), Ice::stringToIdentity("WSTRING2"));

    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
