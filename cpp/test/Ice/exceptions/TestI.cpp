// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace Test;
using namespace std;

ThrowerI::ThrowerI() = default;

void
ThrowerI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

bool
ThrowerI::supportsUndeclaredExceptions(const Ice::Current&)
{
    return true;
}

bool
ThrowerI::supportsAssertException(const Ice::Current&)
{
    return false;
}

void
ThrowerI::throwAasA(int32_t a, const Ice::Current&)
{
    throw A{a};
}

void
ThrowerI::throwAorDasAorD(int32_t a, const Ice::Current&)
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

void
ThrowerI::throwBasA(int32_t a, int32_t b, const Ice::Current& current)
{
    throwBasB(a, b, current);
}

void
ThrowerI::throwCasA(int32_t a, int32_t b, int32_t c, const Ice::Current& current)
{
    throwCasC(a, b, c, current);
}

void
ThrowerI::throwBasB(int32_t a, int32_t b, const Ice::Current&)
{
    throw B{a, b};
}

void
ThrowerI::throwCasB(int32_t a, int32_t b, int32_t c, const Ice::Current& current)
{
    throwCasC(a, b, c, current);
}

void
ThrowerI::throwCasC(int32_t a, int32_t b, int32_t c, const Ice::Current&)
{
    throw C{a, b, c};
}

void
ThrowerI::throwModA(int32_t a, int32_t a2, const Ice::Current&)
{
    throw Mod::A{a, a2};
}

void
ThrowerI::throwUndeclaredA(int32_t a, const Ice::Current&)
{
    throw A{a};
}

void
ThrowerI::throwUndeclaredB(int32_t a, int32_t b, const Ice::Current&)
{
    throw B{a, b};
}

void
ThrowerI::throwUndeclaredC(int32_t a, int32_t b, int32_t c, const Ice::Current&)
{
    throw C{a, b, c};
}

void
ThrowerI::throwLocalException(const Ice::Current&)
{
    throw Ice::TimeoutException{__FILE__, __LINE__, "thrower throwing timeout exception"};
}

void
ThrowerI::throwNonIceException(const Ice::Current&)
{
    throw int(12345);
}

void
ThrowerI::throwAssertException(const Ice::Current&)
{
    assert(false); // Not supported in C++.
}

Ice::ByteSeq
ThrowerI::throwMemoryLimitException(Ice::ByteSeq, const Ice::Current&)
{
    return Ice::ByteSeq(1024 * 20); // 20 KB.
}

void
ThrowerI::throwLocalExceptionIdempotent(const Ice::Current&)
{
    throw Ice::TimeoutException{__FILE__, __LINE__, "thrower throwing timeout exception"};
}

void
ThrowerI::throwDispatchException(uint8_t replyStatus, const Ice::Current&)
{
    throw Ice::DispatchException{__FILE__, __LINE__, Ice::ReplyStatus{replyStatus}};
}

void
ThrowerI::throwAfterResponse(const Ice::Current&)
{
    //
    // Only relevant for AMD.
    //
}

void
ThrowerI::throwAfterException(const Ice::Current&)
{
    //
    // Only relevant for AMD.
    //
    throw A(12345);
}
