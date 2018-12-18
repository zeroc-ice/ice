// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SESSION_I_H
#define SESSION_I_H

#include <Test.h>
#include <TestControllerI.h>

// XXX

//
// The session manager essentially has to be the test controller since
// it will be the entity that is notified of session creation. Can it be
// notified of session destruction as well? There really isn't any
// reason why not as long as there isn't a deadlock situation.
//

class SessionManagerI : public Glacier2::SessionManager
{
public:
    SessionManagerI(const TestControllerIPtr&);

    virtual Glacier2::SessionPrx create(const std::string&, const Glacier2::SessionControlPrx&, const Ice::Current&);

private:
    TestControllerIPtr _controller;
};

class SessionI : public Test::TestSession
{
public:

    SessionI(const Glacier2::SessionControlPrx&, const TestControllerIPtr&);
    virtual void destroySession(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    virtual void destroy(const Ice::Current&);

private:

    Glacier2::SessionControlPrx _sessionControl;
    TestControllerIPtr _controller;
};

#endif
