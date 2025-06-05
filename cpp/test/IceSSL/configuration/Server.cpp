// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

#include <stdexcept>

using namespace std;

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    string testdir;
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
    if (argc < 2)
    {
        ostringstream os;
        os << "Usage: " << argv[0] << " testdir";
        throw invalid_argument(os.str());
    }
    testdir = string(argv[1]);
#else
    testdir = "certs/configuration";
#endif

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint("tcp"));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::Identity id = Ice::stringToIdentity("factory");
    adapter->add(make_shared<ServerFactoryI>(testdir), id);
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
