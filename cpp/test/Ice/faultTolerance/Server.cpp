// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    //
    // In this test, we need a longer server idle time, otherwise
    // our test servers may time out before they are used in the
    // test.
    //
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.ServerIdleTime", "120"); // Two minutes.

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    int port = 0;
    for(int i = 1; i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            ostringstream os;
            os << ": unknown option `" << argv[i] << "'";
            throw invalid_argument(os.str());
        }

        if(port > 0)
        {
            throw runtime_error("only one port can be specified");
        }

        port = atoi(argv[i]);
    }

    if(port <= 0)
    {
        throw runtime_error("no port specified");
    }

    ostringstream endpts;
    endpts << getTestEndpoint(port);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", endpts.str());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = ICE_MAKE_SHARED(TestI);
    adapter->add(object, Ice::stringToIdentity("test"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
