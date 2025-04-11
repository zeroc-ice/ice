// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#if defined(_MSC_VER) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("testtransport"))
#endif

using namespace std;
using namespace Test;

extern "C"
{
    Ice::Plugin* createTestTransport(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
};

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);

    //
    // For this test, we want to disable retries.
    //
    properties->setProperty("Ice.RetryIntervals", "-1");

    //
    // This test kills connections, so we don't want warnings.
    //
    properties->setProperty("Ice.Warn.Connections", "0");

    //
    // This test relies on filling the TCP send/recv buffer, so
    // we rely on a fixed value for these buffers.
    //
    properties->setProperty("Ice.TCP.SndSize", "50000");

    Ice::InitializationData initData;
    initData.properties = properties;

    //
    // Setup the test transport plug-in.
    //
    if (IceInternal::isMinBuild())
    {
        initData.pluginFactories = {{"Test", createTestTransport}};
    }
    else
    {
        properties->setProperty("Ice.Plugin.Test", "TestTransport:createTestTransport");
    }

    string defaultProtocol = properties->getIceProperty("Ice.Default.Protocol");
    properties->setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);

    Ice::CommunicatorHolder communicator = initialize(argc, argv, std::move(initData));
    BackgroundPrx allTests(Test::TestHelper*);
    BackgroundPrx background = allTests(this);
    background->shutdown();
}

DEFINE_TEST(Client)
