// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <CallbackI.h>

using namespace std;
using namespace Ice;
using namespace Test;

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
    communicator()->getProperties()->setProperty("CallbackAdapter.Endpoints", "tcp -p 12010 -t 10000");
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("CallbackAdapter");
    adapter->add(new CallbackI(), stringToIdentity("c1/callback")); // The test allows "c1" as category.
    adapter->add(new CallbackI(), stringToIdentity("c2/callback")); // The test allows "c2" as category.
    adapter->add(new CallbackI(), stringToIdentity("c3/callback")); // The test rejects "c3" as category.
    adapter->add(new CallbackI(), stringToIdentity("_userid/callback")); // The test allows the prefixed userid.
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
