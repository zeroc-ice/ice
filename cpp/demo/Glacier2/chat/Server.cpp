// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Glacier2/PermissionsVerifier.h>

#include <ChatSessionI.h>

using namespace std;
using namespace Ice;
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

class ChatRoomSessionManagerI : public Glacier2::SessionManager
{
public:

    ChatRoomSessionManagerI() :
	_members(new ChatRoomMembers())
    {
    }

    virtual Glacier2::SessionPrx
    create(const string& userId, const ::Ice::Current& current)
    {
	return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(new ChatSessionI(_members, userId)));
    }

private:

    ChatRoomMembersPtr _members;
};

class ChatSessionServer : public Application
{
public:

    virtual int
    run(int, char*[])
    {
	Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("ChatServer");
	
	adapter->add(new DummyPermissionsVerifierI, Ice::stringToIdentity("verifier"));
	adapter->add(new ChatRoomSessionManagerI, Ice::stringToIdentity("ChatRoomSessionManager"));
	adapter->activate();
	communicator()->waitForShutdown();
	
	return EXIT_SUCCESS;
    }
};

int
main(int argc, char* argv[])
{
    ChatSessionServer app;
    return app.main(argc, argv, "config.server");
}
