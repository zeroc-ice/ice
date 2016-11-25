// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
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
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    Ice::InitializationData initData = getTestInitData(argc, argv);

    initData.properties->setProperty("Ice.Warn.Connections", "0");
    initData.properties->setProperty("Ice.Warn.Dispatch", "0");
    initData.properties->setProperty("Ice.ThreadPool.Server.Serialize", "1");

    CallbackServer app;
    return app.main(argc, argv, initData);
}

int
CallbackServer::run(int, char**)
{
    communicator()->getProperties()->setProperty("CallbackAdapter.Endpoints", getTestEndpoint(communicator(), 0));
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("CallbackAdapter");
    adapter->add(new CallbackI(), Ice::stringToIdentity("c/callback"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
