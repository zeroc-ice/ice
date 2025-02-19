// Copyright (c) ZeroC, Inc.

#include "TestAMDI.h"
#include "Ice/Ice.h"

using namespace Test;
using namespace std;

bool
endsWith(const string& s, const string& findme)
{
    if (s.length() > findme.length())
    {
        return 0 == s.compare(s.length() - findme.length(), findme.length(), findme);
    }
    return false;
}

ThrowerI::ThrowerI() = default;

void
ThrowerI::shutdownAsync(function<void()> response, function<void(exception_ptr)>, const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}

void
ThrowerI::supportsUndeclaredExceptionsAsync(
    function<void(bool)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(true);
}

void
ThrowerI::supportsAssertExceptionAsync(
    function<void(bool)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(false);
}

void
ThrowerI::throwAasAAsync(int a, function<void()>, function<void(exception_ptr)> exception, const Ice::Current&)
{
    try
    {
        throw A{a};
    }
    catch (...)
    {
        exception(current_exception());
    }
}

void
ThrowerI::throwAorDasAorDAsync(int a, function<void()>, function<void(exception_ptr)> exception, const Ice::Current&)
{
    try
    {
        if (a > 0)
        {
            throw A{a};
        }
        else
        {
            throw D{a};
        }
    }
    catch (...)
    {
        exception(current_exception());
    }
}

void
ThrowerI::throwBasAAsync(int a, int b, function<void()>, function<void(exception_ptr)> exception, const Ice::Current&)
{
    try
    {
        throw B{a, b};
    }
    catch (...)
    {
        exception(current_exception());
    }
}

void
ThrowerI::throwCasAAsync(
    int a,
    int b,
    int c,
    function<void()>,
    function<void(exception_ptr)> exception,
    const Ice::Current&)
{
    try
    {
        throw C{a, b, c};
    }
    catch (...)
    {
        exception(current_exception());
    }
}

void
ThrowerI::throwBasBAsync(int a, int b, function<void()>, function<void(exception_ptr)> exception, const Ice::Current&)
{
    try
    {
        throw B{a, b};
    }
    catch (...)
    {
        exception(current_exception());
    }
}

void
ThrowerI::throwCasBAsync(
    int a,
    int b,
    int c,
    function<void()>,
    function<void(exception_ptr)> exception,
    const Ice::Current&)
{
    try
    {
        throw C{a, b, c};
    }
    catch (...)
    {
        exception(current_exception());
    }
}

void
ThrowerI::throwCasCAsync(
    int a,
    int b,
    int c,
    function<void()>,
    function<void(exception_ptr)> exception,
    const Ice::Current&)
{
    try
    {
        throw C{a, b, c};
    }
    catch (...)
    {
        exception(current_exception());
    }
}

void
ThrowerI::throwModAAsync(int a, int a2, function<void()>, function<void(exception_ptr)>, const Ice::Current&)
{
    throw Mod::A{a, a2};
}

void
ThrowerI::throwUndeclaredAAsync(int a, function<void()>, function<void(exception_ptr)> exception, const Ice::Current&)
{
    try
    {
        throw A{a};
    }
    catch (...)
    {
        exception(current_exception());
    }
}

void
ThrowerI::throwUndeclaredBAsync(int a, int b, function<void()>, function<void(exception_ptr)>, const Ice::Current&)
{
    throw B{a, b};
}

void
ThrowerI::throwUndeclaredCAsync(
    int a,
    int b,
    int c,
    function<void()>,
    function<void(exception_ptr)> exception,
    const Ice::Current&)
{
    try
    {
        throw C{a, b, c};
    }
    catch (...)
    {
        exception(current_exception());
    }
}

void
ThrowerI::throwLocalExceptionAsync(function<void()>, function<void(exception_ptr)> exception, const Ice::Current&)
{
    try
    {
        throw Ice::TimeoutException{__FILE__, __LINE__, "thrower throwing timeout exception"};
    }
    catch (...)
    {
        exception(current_exception());
    }
}

void
ThrowerI::throwNonIceExceptionAsync(function<void()>, function<void(exception_ptr)> exception, const Ice::Current&)
{
    try
    {
        throw int(12345);
    }
    catch (...)
    {
        exception(current_exception());
    }
}

void
ThrowerI::throwAssertExceptionAsync(function<void()>, function<void(exception_ptr)>, const Ice::Current&)
{
    assert(false); // No supported in C++
}

void
ThrowerI::throwMemoryLimitExceptionAsync(
    Ice::ByteSeq,
    function<void(const Ice::ByteSeq&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(Ice::ByteSeq(1024 * 20)); // 20 KB.
}

void
ThrowerI::throwLocalExceptionIdempotentAsync(function<void()>, function<void(exception_ptr)> error, const Ice::Current&)
{
    error(make_exception_ptr(Ice::TimeoutException{__FILE__, __LINE__, "thrower throwing timeout exception"}));
}

void
ThrowerI::throwDispatchExceptionAsync(
    uint8_t replyStatus,
    function<void()>,
    function<void(exception_ptr)> error,
    const Ice::Current&)
{
    error(make_exception_ptr(Ice::DispatchException{__FILE__, __LINE__, Ice::ReplyStatus{replyStatus}}));
}

void
ThrowerI::throwAfterResponseAsync(function<void()> response, function<void(exception_ptr)>, const Ice::Current&)
{
    response();

    throw std::string();
}

void
ThrowerI::throwAfterExceptionAsync(function<void()>, function<void(exception_ptr)> exception, const Ice::Current&)
{
    try
    {
        throw A(12345);
    }
    catch (...)
    {
        exception(current_exception());
    }
    throw std::string();
}

void
ThrowerI::throwEAsync(function<void()>, function<void(exception_ptr)> exception, const Ice::Current&)
{
    try
    {
        throw E("E");
    }
    catch (...)
    {
        exception(current_exception());
    }
}
