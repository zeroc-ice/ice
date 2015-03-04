// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
SessionServer::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("SessionFactory");

    IceUtil::TimerPtr timer = new IceUtil::Timer();
    ReapTaskPtr reapTask = new ReapTask;
    timer->scheduleRepeated(reapTask, IceUtil::Time::seconds(1));

    SessionFactoryPtr factory = new SessionFactoryI(reapTask);
    adapter->add(factory, communicator()->stringToIdentity("SessionFactory"));
    adapter->activate();
    communicator()->waitForShutdown();

    timer->destroy();

    return EXIT_SUCCESS;
}
