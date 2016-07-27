// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
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
    return app.main(argc, argv);
}

int
SessionControlServer::run(int, char**)
{
    communicator()->getProperties()->setProperty("SessionControlAdapter.Endpoints", "tcp -p 12010");
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("SessionControlAdapter");
    adapter->add(new SessionManagerI, Ice::stringToIdentity("SessionManager"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
