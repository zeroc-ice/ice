// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <SessionI.h>

using namespace std;

class SessionServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    SessionServer app;
    return app.main(argc, argv, "config.sessionserver");
}

int
SessionServer::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("SessionServer");
    Glacier2::PermissionsVerifierPtr dpv = new DummyPermissionsVerifierI;
    adapter->add(dpv, communicator()->stringToIdentity("verifier"));
    Glacier2::SessionManagerPtr sm = new SessionManagerI;
    adapter->add(sm, communicator()->stringToIdentity("sessionmanager"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
