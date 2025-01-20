// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "StringConverterI.h"
#include "TestAMDI.h"
#include "TestHelper.h"
#include "WstringAMDI.h"

using namespace std;

class ServerAMD : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
ServerAMD::run(int argc, char** argv)
{
    setProcessStringConverter(make_shared<Test::StringConverterI>());
    setProcessWstringConverter(make_shared<Test::WstringConverterI>());

    Ice::CommunicatorHolder communicator = initialize(argc, argv);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("TEST"));
    adapter->add(make_shared<Test1::WstringClassI>(), Ice::stringToIdentity("WSTRING1"));
    adapter->add(make_shared<Test2::WstringClassI>(), Ice::stringToIdentity("WSTRING2"));

    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(ServerAMD)
