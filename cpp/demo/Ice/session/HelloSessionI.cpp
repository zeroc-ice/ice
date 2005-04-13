// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>

#include <HelloSessionI.h>

using namespace std;

HelloSessionI::HelloSessionI(const SessionManagerIPtr& manager) :
    _manager(manager)
{
}

HelloSessionI::~HelloSessionI()
{
}

void
HelloSessionI::sayHello(const Ice::Current&) const
{
    cout << "Hello World!" << endl;
}

//
// Destroy all session specific state.
//
void
HelloSessionI::destroyed(const Ice::Current& c)
{
    c.adapter->remove(c.id);
}

//
// This method is called by the client to destroy a session. All
// it should do is call remove on the session manager. All user
// specific cleanup should go in the destroyed() callback.
//
void
HelloSessionI::destroy(const Ice::Current& c)
{
    _manager->remove(c.id);
}

void
HelloSessionI::refresh(const Ice::Current& c)
{
    _manager->refresh(c.id);
}
