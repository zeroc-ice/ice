// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ThroughputI.h>

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
    return app.main(argc, argv, "config.server");
}

int
ThroughputServer::run(int argc, char* argv[])
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Throughput");
    adapter->add(new ThroughputI, communicator()->stringToIdentity("throughput"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
