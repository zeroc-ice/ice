// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
    communicator()->getProperties()->setProperty("CallbackAdapter.Endpoints", "tcp -p 12345 -t 2000");
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("CallbackAdapter");
    CallbackPrx self = CallbackPrx::uncheckedCast(adapter->createProxy(Ice::stringToIdentity("callback")));
    adapter->add(new CallbackI(communicator()), Ice::stringToIdentity("callback"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
