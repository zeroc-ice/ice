//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

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
    auto properties = communicator->getProperties();

    string name = properties->getProperty("Ice.ProgramName");

    auto adapter = communicator->createObjectAdapter("Server");
    adapter->add(make_shared<TestI>(properties), Ice::stringToIdentity("allocatable"));
    adapter->add(make_shared<TestI>(properties), Ice::stringToIdentity("nonallocatable"));
    adapter->add(make_shared<TestI>(properties), Ice::stringToIdentity("allocatable1"));
    adapter->add(make_shared<TestI>(properties), Ice::stringToIdentity("allocatable2"));
    adapter->add(make_shared<TestI>(properties), Ice::stringToIdentity("allocatable3"));
    adapter->add(make_shared<TestI>(properties), Ice::stringToIdentity("allocatable4"));
    adapter->add(make_shared<TestI>(properties), Ice::stringToIdentity("allocatable11"));
    adapter->add(make_shared<TestI>(properties), Ice::stringToIdentity("allocatable21"));
    adapter->add(make_shared<TestI>(properties), Ice::stringToIdentity("allocatable31"));
    adapter->add(make_shared<TestI>(properties), Ice::stringToIdentity("allocatable41"));

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
