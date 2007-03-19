// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Latency.h>

using namespace std;
using namespace Demo;

class LatencyServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    LatencyServer app;
    return app.main(argc, argv, "config.server");
}

int
LatencyServer::run(int argc, char* argv[])
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Latency");
    Ice::ObjectPtr object = new Ping;
    adapter->add(new Ping, communicator()->stringToIdentity("ping"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
