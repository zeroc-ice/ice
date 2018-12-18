// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <SessionI.h>

using namespace std;
using namespace Test;

SessionManagerI::SessionManagerI(const TestControllerIPtr& controller):
    _controller(controller)
{
}

Glacier2::SessionPrx
SessionManagerI::create(const string&, const Glacier2::SessionControlPrx& sessionControl, const Ice::Current& current)
{
    Glacier2::SessionPrx newSession = Glacier2::SessionPrx::uncheckedCast(
        current.adapter->addWithUUID(new SessionI(sessionControl, _controller)));
    _controller->addSession(SessionTuple(newSession, sessionControl));
    return newSession;
}

SessionI::SessionI(const Glacier2::SessionControlPrx& sessionControl, const TestControllerIPtr& controller) :
    _sessionControl(sessionControl),
    _controller(controller)
{
    assert(sessionControl);
}

void
SessionI::destroySession(const Ice::Current&)
{
    _sessionControl->destroy();
}

void
SessionI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
SessionI::destroy(const Ice::Current& current)
{
    _controller->notifyDestroy(_sessionControl);
    current.adapter->remove(current.id);
}
