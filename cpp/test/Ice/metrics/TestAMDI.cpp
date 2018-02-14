// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestAMDI.h>

void
MetricsI::op_async(const Test::AMD_Metrics_opPtr& cb, const Ice::Current&)
{
    cb->ice_response();
}

void
MetricsI::fail_async(const Test::AMD_Metrics_failPtr& cb, const Ice::Current& current)
{
    current.con->close(true);
    cb->ice_response();
}

void
MetricsI::opWithUserException_async(const Test::AMD_Metrics_opWithUserExceptionPtr& cb, const Ice::Current&)
{
    cb->ice_exception(Test::UserEx());
}

void
MetricsI::opWithRequestFailedException_async(const Test::AMD_Metrics_opWithRequestFailedExceptionPtr& cb, 
                                             const Ice::Current&)
{
    cb->ice_exception(Ice::ObjectNotExistException(__FILE__, __LINE__));
}

void
MetricsI::opWithLocalException_async(const Test::AMD_Metrics_opWithLocalExceptionPtr& cb, const Ice::Current&)
{
    cb->ice_exception(Ice::SyscallException(__FILE__, __LINE__));
}

void
MetricsI::opWithUnknownException_async(const Test::AMD_Metrics_opWithUnknownExceptionPtr&, const Ice::Current&)
{
    throw "Test";
}

void
MetricsI::opByteS_async(const Test::AMD_Metrics_opByteSPtr& cb, const Test::ByteSeq&, const Ice::Current&)
{
    cb->ice_response();
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
