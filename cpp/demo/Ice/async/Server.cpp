// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HelloI.h>
#include <WorkQueue.h>

using namespace std;

class AsyncServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
    virtual void interruptCallback(int);

private:

    WorkQueuePtr _workQueue;
};

int
main(int argc, char* argv[])
{
    AsyncServer app;
    return app.main(argc, argv, "config.server");
}

int
AsyncServer::run(int argc, char* argv[])
{
    callbackOnInterrupt();

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Hello");
    _workQueue = new WorkQueue();
    adapter->add(new HelloI(_workQueue), communicator()->stringToIdentity("hello"));

    _workQueue->start();
    adapter->activate();

    communicator()->waitForShutdown();
    _workQueue->getThreadControl().join();
    return EXIT_SUCCESS;
}

void
AsyncServer::interruptCallback(int)
{
    _workQueue->destroy();
    communicator()->shutdown();
}
