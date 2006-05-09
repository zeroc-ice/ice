// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

class DummyPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool
    checkPermissions(const string&, const string&, string&, const Ice::Current&) const
    {
        return true;
    }
};

class SessionControlServer : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    SessionControlServer app;
    return app.main(argc, argv);
}

int
SessionControlServer::run(int argc, char* argv[])
{
    communicator()->getProperties()->setProperty("SessionControlAdapter.Endpoints", "tcp -p 12010 -t 10000");
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("SessionControlAdapter");
    adapter->add(new DummyPermissionsVerifierI, communicator()->stringToIdentity("verifier"));
    adapter->add(new SessionManagerI, communicator()->stringToIdentity("SessionManager"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
