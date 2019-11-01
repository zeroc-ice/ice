//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <SessionI.h>

using namespace std;
using namespace Test;

SessionManagerI::SessionManagerI(const shared_ptr<TestControllerI>& controller):
    _controller(controller)
{
}

shared_ptr<Glacier2::SessionPrx>
SessionManagerI::create(string, shared_ptr<Glacier2::SessionControlPrx> sessionControl, const Ice::Current& current)
{
    auto newSession = Ice::uncheckedCast<Glacier2::SessionPrx>(
        current.adapter->addWithUUID(make_shared<SessionI>(sessionControl, _controller)));
    _controller->addSession(SessionTuple(newSession, move(sessionControl)));
    return newSession;
}

SessionI::SessionI(shared_ptr<Glacier2::SessionControlPrx> sessionControl,
                   shared_ptr<TestControllerI> controller) :
    _sessionControl(move(sessionControl)),
    _controller(move(controller))
{
    assert(_sessionControl);
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
