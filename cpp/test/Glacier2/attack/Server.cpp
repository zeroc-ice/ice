// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <BackendI.h>

using namespace std;
using namespace Ice;
using namespace Test;

class BackendServer : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    BackendServer app;
    return app.main(argc, argv);
}

int
BackendServer::run(int argc, char* argv[])
{
    communicator()->getProperties()->setProperty("BackendAdapter.Endpoints", "tcp -p 12010 -t 10000");
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("BackendAdapter");
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
