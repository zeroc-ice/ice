// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ThroughputI.h>
#include <Ice/Application.h>

using namespace std;

class ThroughputServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    ThroughputServer app;
    return app.main(argc, argv, "config");
}

int
ThroughputServer::run(int argc, char* argv[])
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Throughput");
    adapter->add(new ThroughputI, Ice::stringToIdentity("throughput"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
