// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
    Ice::CommunicatorHolder communicator = initialize(argc, argv);

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    string id = communicator->getProperties()->getPropertyWithDefault("Identity", "test");
    adapter->add(ICE_MAKE_SHARED(TestI), Ice::stringToIdentity(id));

    try
    {
        adapter->activate();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
