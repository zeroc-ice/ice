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

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicatorHolder = initialize(argc, argv);
    auto properties = communicatorHolder->getProperties();
    auto name = properties->getProperty("Ice.ProgramName");

    auto adapter = communicatorHolder->createObjectAdapter("Server");
    adapter->add(make_shared<TestI>(move(properties)), Ice::stringToIdentity(name));

    try
    {
        adapter->activate();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    communicatorHolder->waitForShutdown();
}

DEFINE_TEST(Server)
