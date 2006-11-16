// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <QueueI.h>
#include <Ice/Application.h>

using namespace std;

class QueueServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    QueueServer app;
    return app.main(argc, argv, "config.server");
}

int
QueueServer::run(int argc, char* argv[])
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Queue");
    adapter->add(new QueueI, communicator()->stringToIdentity("queue"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
