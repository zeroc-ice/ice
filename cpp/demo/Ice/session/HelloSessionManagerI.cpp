// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HelloSessionManagerI.h>

#include <HelloSession.h>

using namespace std;
using namespace Demo;

class HelloSessionI : public HelloSession
{
public:

    HelloSessionI(const SessionManagerIPtr& manager) :
	_manager(manager)
    {
    }

    ~HelloSessionI()
    {
    }

    virtual void
    sayHello(const Ice::Current&) const
    {
	cout << "Hello World!" << endl;
    }
    
    // Common session specific code.

    //
    // Destroy all session specific state.
    //
    virtual void
    destroyed(const Ice::Current& c)
    {
    	c.adapter->remove(c.id);
    }

    //
    // This method is called by the client to destroy a session. All
    // it should do is call remove on the session manager. All user
    // specific cleanup should go in the destroyed() callback.
    //
    virtual void
    destroy(const Ice::Current& c)
    {
    	_manager->remove(c.id);
    }

    virtual void
    refresh(const Ice::Current& c)
    {
    	_manager->refresh(c.id);
    }

private:

    const SessionManagerIPtr _manager;
};

SessionPrx
HelloSessionManagerI::create(const Ice::Current& c)
{
    SessionPrx session = SessionPrx::uncheckedCast(c.adapter->addWithUUID(new HelloSessionI(this)));
    add(session);
    return session;
}
