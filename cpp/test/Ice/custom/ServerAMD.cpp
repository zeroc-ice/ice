// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestAMDI.h>
#include <WstringAMDI.h>
#include <StringConverterI.h>

using namespace std;

class ServerAMD : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
ServerAMD::run(int argc, char** argv)
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

DEFINE_TEST(ServerAMD)
