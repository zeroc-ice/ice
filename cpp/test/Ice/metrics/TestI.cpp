// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

void
MetricsI::op(const Ice::Current&)
{
}

void
MetricsI::fail(const Ice::Current& current)
{
    current.con->close(true);
}

void
MetricsI::opWithUserException(const Ice::Current&)
{
    throw Test::UserEx();
}

void
MetricsI::opWithRequestFailedException(const Ice::Current&)
{
    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
}

void
MetricsI::opWithLocalException(const Ice::Current&)
{
    throw Ice::SyscallException(__FILE__, __LINE__);
}

void
MetricsI::opWithUnknownException(const Ice::Current&)
{
    throw "TEST";
}

void
MetricsI::opByteS(const Test::ByteSeq&, const Ice::Current&)
{
}

Ice::ObjectPrx
MetricsI::getAdmin(const Ice::Current& current)
{
    return current.adapter->getCommunicator()->getAdmin();
}

void
MetricsI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

ControllerI::ControllerI(const Ice::ObjectAdapterPtr& adapter) : _adapter(adapter)
{
}

void
ControllerI::hold(const Ice::Current&)
{
    _adapter->hold();
    _adapter->waitForHold();
}

void
ControllerI::resume(const Ice::Current&)
{
    _adapter->activate();
}
