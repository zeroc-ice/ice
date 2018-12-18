// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

//
// Server side is pure unicode
//
class MyObjectI : public Test::MyObject
{
public:

    virtual wstring widen(ICE_IN(string) msg, const Ice::Current&)
    {
        return stringToWstring(msg, Ice::getProcessStringConverter(),
                               Ice::getProcessWstringConverter());
    }

    virtual string narrow(ICE_IN(wstring) wmsg, const Ice::Current&)
    {
        return wstringToString(wmsg, Ice::getProcessStringConverter(),
                               Ice::getProcessWstringConverter());
    }

    virtual void shutdown(const Ice::Current& current)
    {
        current.adapter->getCommunicator()->shutdown();
    }
};

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(MyObjectI), Ice::stringToIdentity("test"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
