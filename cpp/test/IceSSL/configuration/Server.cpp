//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

#if defined(ICE_USE_OPENSSL)
#  include <IceSSL/OpenSSL.h>
#endif

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    string testdir;
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
    if(argc < 2)
    {
        ostringstream os;
        os << "Usage: " << argv[0] << " testdir";
        throw invalid_argument(os.str());
    }
    testdir = string(argv[1]) + "/../certs";
#else
    testdir = "certs";
#endif

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint("tcp"));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::Identity id = Ice::stringToIdentity("factory");
    adapter->add(ICE_MAKE_SHARED(ServerFactoryI, testdir), id);
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
