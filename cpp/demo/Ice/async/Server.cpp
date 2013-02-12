// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
AsyncServer::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    callbackOnInterrupt();

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Hello");
    _workQueue = new WorkQueue();
    Demo::HelloPtr hello = new HelloI(_workQueue);
    adapter->add(hello, communicator()->stringToIdentity("hello"));

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
