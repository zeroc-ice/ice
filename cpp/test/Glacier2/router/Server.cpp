// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
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
    Ice::InitializationData initData = getTestInitData(argc, argv);

    initData.properties->setProperty("Ice.Warn.Connections", "0");
    initData.properties->setProperty("Ice.Warn.Dispatch", "0");

    CallbackServer app;
    return app.main(argc, argv, initData);
}

int
CallbackServer::run(int, char**)
{
    communicator()->getProperties()->setProperty("CallbackAdapter.Endpoints", getTestEndpoint(communicator(), 0));
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("CallbackAdapter");
    adapter->add(new CallbackI(), Ice::stringToIdentity("c1/callback")); // The test allows "c1" as category.
    adapter->add(new CallbackI(), Ice::stringToIdentity("c2/callback")); // The test allows "c2" as category.
    adapter->add(new CallbackI(), Ice::stringToIdentity("c3/callback")); // The test rejects "c3" as category.
    adapter->add(new CallbackI(), Ice::stringToIdentity("_userid/callback")); // The test allows the prefixed userid.
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
