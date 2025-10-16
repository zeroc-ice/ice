// Copyright (c) ZeroC, Inc.

#include "TestAMDI.h"
#include "Ice/Ice.h"

using namespace std;

void
MetricsI::opAsync(function<void()> response, function<void(exception_ptr)>, const Ice::Current&)
{
    response();
}

void
MetricsI::failAsync(function<void()> response, function<void(exception_ptr)>, const Ice::Current& current)
{
    current.con->abort();
    response();
}

void
MetricsI::opWithUserExceptionAsync(function<void()>, function<void(exception_ptr)> error, const Ice::Current&)
{
    try
    {
        throw Test::UserEx();
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
MetricsI::opWithRequestFailedExceptionAsync(function<void()>, function<void(exception_ptr)> error, const Ice::Current&)
{
    try
    {
        throw Ice::ObjectNotExistException{__FILE__, __LINE__};
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
MetricsI::opWithLocalExceptionAsync(function<void()>, function<void(exception_ptr)> error, const Ice::Current&)
{
    error(make_exception_ptr(Ice::SyscallException{__FILE__, __LINE__, "opWithLocalException simulated failure", 0}));
}

void
MetricsI::opWithUnknownExceptionAsync(function<void()>, function<void(exception_ptr)>, const Ice::Current&)
{
    throw "Test";
}

void
MetricsI::opByteSAsync(Test::ByteSeq, function<void()> response, function<void(exception_ptr)>, const Ice::Current&)
{
    response();
}

void
MetricsI::getAdminAsync(
    std::function<void(const std::optional<Ice::ObjectPrx>&)> response,
    std::function<void(exception_ptr)>,
    const Ice::Current& current)
{
    response(current.adapter->getCommunicator()->getAdmin());
}

void
MetricsI::shutdownAsync(std::function<void()> response, std::function<void(exception_ptr)>, const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}

ControllerI::ControllerI(Ice::ObjectAdapterPtr adapter) : _adapter(std::move(adapter)) {}

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
