//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

using namespace std;

class Server final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

class TestActivationFailure final : public std::runtime_error
{
public:

    TestActivationFailure(const string& what_arg) :
        runtime_error(what_arg)
    {
    }
};

void
Server::run(int argc, char** argv)
{
    auto properties = createTestProperties(argc, argv);
    if(properties->getPropertyAsInt("FailOnStartup") > 0)
    {
        throw TestActivationFailure("FailOnStartup");
    }
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    auto adapter = communicator->createObjectAdapter("TestAdapter");
    auto testI = make_shared<TestI>();
    adapter->add(testI, Ice::stringToIdentity(properties->getProperty("Ice.Admin.ServerId")));

    int delay = properties->getPropertyAsInt("ActivationDelay");
    if(delay > 0)
    {
        this_thread::sleep_for(chrono::seconds(delay));
    }

    try
    {
        adapter->activate();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    communicator->waitForShutdown();

    delay = properties->getPropertyAsInt("DeactivationDelay");
    if(delay > 0)
    {
        this_thread::sleep_for(chrono::seconds(delay));
    }

    if(testI->isFailed())
    {
        throw TestActivationFailure("test failed");
    }
}

int
main(int argc, char** argv)
{
    int status = 0;
    try
    {
        Server server;
        server.run(argc, argv);
    }
    catch(const TestActivationFailure&)
    {
        status = 1; // excpected failure
    }
    catch(const std::exception& ex)
    {
        std::cerr << "error: " << ex.what() << std::endl;
        status = 1;
    }
    return status;
}
