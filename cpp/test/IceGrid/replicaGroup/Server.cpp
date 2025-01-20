// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

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
    auto adpt = communicator->createObjectAdapter("ReplicatedAdapter");
    auto properties = communicator->getProperties();
    auto object = make_shared<TestI>(properties);
    adpt->add(object, Ice::stringToIdentity(properties->getIceProperty("Ice.ProgramName")));
    adpt->add(object, Ice::stringToIdentity(properties->getProperty("Identity")));
    try
    {
        adpt->activate();
        communicator->getAdmin();
    }
    catch (const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
        //
        // getAdmin might raise this if communicator is shutdown by
        // servant.
        //
    }
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
