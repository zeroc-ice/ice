// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Glacier2/PermissionsVerifier.h>
#include <SessionI.h>

using namespace std;
using namespace Ice;
using namespace Test;

class SessionControlServer : public Application
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

    SessionControlServer app;
    Ice::InitializationData initData = getTestInitData(argc, argv);
    return app.main(argc, argv, initData);
}

int
SessionControlServer::run(int, char**)
{
    communicator()->getProperties()->setProperty("SessionControlAdapter.Endpoints", getTestEndpoint(communicator(), 0));
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("SessionControlAdapter");
    adapter->add(new SessionManagerI, Ice::stringToIdentity("SessionManager"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
