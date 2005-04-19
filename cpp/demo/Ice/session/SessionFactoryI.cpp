// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h> // XXX Get rid of this, SessionFactoryI.cpp must include this.
#include <SessionFactoryI.h>
#include <ReapThread.h>

using namespace std;
using namespace Demo;

SessionFactoryI::SessionFactoryI()
{
}

SessionFactoryI::~SessionFactoryI()
{
}

SessionPrx
SessionFactoryI::create(const Ice::Current& c)
{
    Lock sync(*this); // XXX What is the mutex lock needed for?

    SessionIPtr session = new SessionI;
    SessionPrx proxy = SessionPrx::uncheckedCast(c.adapter->addWithUUID(session));
    ReapThread::instance()->add(proxy, session);
    return proxy;
}

void
SessionFactoryI::shutdown(const Ice::Current& c)
{
    cout << "Shutting down..." << endl;
    c.adapter->getCommunicator()->shutdown();
}
