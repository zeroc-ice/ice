// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ChatSessionI.h>
#include <Ice/Application.h>
#include <Glacier2/PermissionsVerifier.h>

using namespace std;
using namespace Demo;

class DummyPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool
    checkPermissions(const string& userId, const string& passwd, string&, const Ice::Current&) const
    {
	return true;
    }
};

class ChatSessionManagerI : public Glacier2::SessionManager
{
public:

    virtual Glacier2::SessionPrx
    create(const string& userId, const Ice::Current& current)
    {
	return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(new ChatSessionI(userId)));
    }
};

class ChatServer : public Ice::Application
{
public:

    virtual int
    run(int, char*[])
    {
	Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("ChatServer");
	
	adapter->add(new DummyPermissionsVerifierI, Ice::stringToIdentity("verifier"));
	adapter->add(new ChatSessionManagerI, Ice::stringToIdentity("ChatSessionManager"));
	adapter->activate();
	communicator()->waitForShutdown();
	
	return EXIT_SUCCESS;
    }
};

int
main(int argc, char* argv[])
{
    ChatServer app;
    return app.main(argc, argv, "config.server");
}
