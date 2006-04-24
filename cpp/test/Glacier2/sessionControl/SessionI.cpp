// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <SessionI.h>

using namespace std;
using namespace Test;

Glacier2::SessionPrx
SessionManagerI::create(const string&, const Glacier2::SessionControlPrx& sessionControl, const Ice::Current& current)
{
    return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(new SessionI(sessionControl)));
}

SessionI::SessionI(const Glacier2::SessionControlPrx& sessionControl) :
    _sessionControl(sessionControl)
{
    assert(sessionControl);
}

void
SessionI::destroySession(const Ice::Current& current)
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
    current.adapter->remove(current.id);
}
