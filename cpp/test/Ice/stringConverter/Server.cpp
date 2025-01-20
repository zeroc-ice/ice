// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;

//
// Server side is pure unicode
//
class MyObjectI : public Test::MyObject
{
public:
    wstring widen(string msg, const Ice::Current&) override
    {
        return stringToWstring(msg, Ice::getProcessStringConverter(), Ice::getProcessWstringConverter());
    }

    string narrow(wstring wmsg, const Ice::Current&) override
    {
        return wstringToString(wmsg, Ice::getProcessStringConverter(), Ice::getProcessWstringConverter());
    }

    void shutdown(const Ice::Current& current) override { current.adapter->getCommunicator()->shutdown(); }
};

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<MyObjectI>(), Ice::stringToIdentity("test"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
