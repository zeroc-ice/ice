// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#include <Ice/Application.h>
#include <CallbackI.h>

using namespace std;
using namespace Ice;

class CallbackServer : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    CallbackServer app;
    return app.main(argc, argv);
}

int
CallbackServer::run(int argc, char* argv[])
{
    communicator()->getProperties()->setProperty("CallbackAdapter.Endpoints", "tcp -p 12345 -t 10000");
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("CallbackAdapter");
    CallbackPrx self = CallbackPrx::uncheckedCast(adapter->createProxy(Ice::stringToIdentity("callback")));
    adapter->add(new CallbackI(communicator()), Ice::stringToIdentity("callback"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
