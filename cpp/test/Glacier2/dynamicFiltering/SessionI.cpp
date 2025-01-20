// Copyright (c) ZeroC, Inc.

#include "SessionI.h"
#include "Ice/Ice.h"

using namespace std;
using namespace Test;

SessionManagerI::SessionManagerI(const shared_ptr<TestControllerI>& controller) : _controller(controller) {}

optional<Glacier2::SessionPrx>
SessionManagerI::create(string, optional<Glacier2::SessionControlPrx> sessionControl, const Ice::Current& current)
{
    auto newSession =
        current.adapter->addWithUUID<Glacier2::SessionPrx>(make_shared<SessionI>(sessionControl, _controller));
    _controller->addSession(SessionTuple(newSession, std::move(sessionControl)));
    return newSession;
}

SessionI::SessionI(optional<Glacier2::SessionControlPrx> sessionControl, shared_ptr<TestControllerI> controller)
    : _sessionControl(std::move(sessionControl)),
      _controller(std::move(controller))
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
