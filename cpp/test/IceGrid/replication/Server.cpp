// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestCommon.h>

using namespace std;

class Server : public Ice::Application
{
public:

    virtual int run(int argc, char* argv[]);
};

int
Server::run(int, char**)
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = new TestI();
    adapter->add(object, Ice::stringToIdentity(communicator()->getProperties()->getProperty("Identity")));
    shutdownOnInterrupt();
    try
    {
        adapter->activate();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    communicator()->waitForShutdown();
    ignoreInterrupt();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    Server app;
    return app.main(argc, argv);
}
