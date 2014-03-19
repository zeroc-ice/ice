// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>

#include <Callback.h>

using namespace Test;

namespace
{

class SessionHelperServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

class CallbackI : public Callback
{

public:
    
    virtual void
    initiateCallback(const CallbackReceiverPrx& proxy, const Ice::Current& current)
    {
        proxy->callback(current.ctx);
    }

    virtual void
    initiateCallbackEx(const CallbackReceiverPrx& proxy, const Ice::Current& current)
    {
        proxy->callbackEx(current.ctx);
    }

    virtual void
    shutdown(const Ice::Current& current)
    {
        current.adapter->getCommunicator()->shutdown();
    }
};

}

int
SessionHelperServer::run(int, char**)
{
    communicator()->getProperties()->setProperty("CallbackAdapter.Endpoints", "default -p 12010");
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("CallbackAdapter");
    adapter->add(new CallbackI(), communicator()->stringToIdentity("callback"));
    adapter->activate();
    communicator()->waitForShutdown();        

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    SessionHelperServer app;
    return app.main(argc, argv);
}

