// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <SessionFactoryI.h>

using namespace std;
using namespace Demo;

class SessionServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    SessionServer app;
    return app.main(argc, argv, "config.server");
}

int
SessionServer::run(int argc, char* argv[])
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("SessionFactory");

    ReapThreadPtr reaper = new ReapThread();
    reaper->start();

    adapter->add(new SessionFactoryI(reaper), communicator()->stringToIdentity("SessionFactory"));
    adapter->activate();
    communicator()->waitForShutdown();

    reaper->terminate();
    reaper->getThreadControl().join();

    return EXIT_SUCCESS;
}
