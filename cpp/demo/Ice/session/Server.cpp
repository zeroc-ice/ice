// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <SessionFactoryI.h>
#include <ReapThread.h>

using namespace std;
using namespace Demo;

class SessionServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);

private:

    void menu();
    string trim(const string&);
};

int
main(int argc, char* argv[])
{
    SessionServer app;
    return app.main(argc, argv, "config");
}

int
SessionServer::run(int argc, char* argv[])
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("SessionFactory");

    ReapThreadPtr reaper = ReapThread::instance();
    reaper->start();

    adapter->add(new SessionFactoryI, Ice::stringToIdentity("SessionFactory"));
    adapter->activate();
    communicator()->waitForShutdown();

    reaper->terminate();
    reaper->getThreadControl().join();

    return EXIT_SUCCESS;
}
