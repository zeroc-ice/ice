//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Configuration.h"
#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#ifdef _MSC_VER
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
#ifdef ICE_STATIC_LIBS
    Ice::registerPluginFactory("Test", createTestTransport, false);
#endif

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

    //
    // Setup the test transport plug-in.
    //
    properties->setProperty("Ice.Plugin.Test", "TestTransport:createTestTransport");
    string defaultProtocol = properties->getIceProperty("Ice.Default.Protocol");
    properties->setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    BackgroundPrx allTests(Test::TestHelper*);
    BackgroundPrx background = allTests(this);
    background->shutdown();
}

DEFINE_TEST(Client)
