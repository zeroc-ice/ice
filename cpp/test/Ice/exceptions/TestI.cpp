// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestCommon.h>

using namespace Test;
using namespace std;

ThrowerI::ThrowerI()
{
}

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
ThrowerI::throwAasA(Ice::Int a, const Ice::Current&)
{
    A ex;
    ex.aMem = a;
    throw ex;
}

void
ThrowerI::throwAorDasAorD(Ice::Int a, const Ice::Current&)
{
    if(a > 0)
    {
        A ex;
        ex.aMem = a;
        throw ex;
    }
    else
    {
        D ex;
        ex.dMem = a;
        throw ex;
    }
}

void
ThrowerI::throwBasA(Ice::Int a, Ice::Int b, const Ice::Current& current)
{
    throwBasB(a, b, current);
}

void
ThrowerI::throwCasA(Ice::Int a, Ice::Int b, Ice::Int c, const Ice::Current& current)
{
    throwCasC(a, b, c, current);
}

void
ThrowerI::throwBasB(Ice::Int a, Ice::Int b, const Ice::Current&)
{
    B ex;
    ex.aMem = a;
    ex.bMem = b;
    throw ex;
}

void
ThrowerI::throwCasB(Ice::Int a, Ice::Int b, Ice::Int c, const Ice::Current& current)
{
    throwCasC(a, b, c, current);
}

void
ThrowerI::throwCasC(Ice::Int a, Ice::Int b, Ice::Int c, const Ice::Current&)
{
    C ex;
    ex.aMem = a;
    ex.bMem = b;
    ex.cMem = c;
    throw ex;
}

void
ThrowerI::throwModA(Ice::Int a, Ice::Int a2, const Ice::Current&)
{
    Mod::A ex;
    ex.aMem = a;
    ex.a2Mem = a2;
    throw ex;
}

void
ThrowerI::throwUndeclaredA(Ice::Int a, const Ice::Current&)
{
    A ex;
    ex.aMem = a;
    throw ex;
}

void
ThrowerI::throwUndeclaredB(Ice::Int a, Ice::Int b, const Ice::Current&)
{
    B ex;
    ex.aMem = a;
    ex.bMem = b;
    throw ex;
}

void
ThrowerI::throwUndeclaredC(Ice::Int a, Ice::Int b, Ice::Int c, const Ice::Current&)
{
    C ex;
    ex.aMem = a;
    ex.bMem = b;
    ex.cMem = c;
    throw ex;
}

void
ThrowerI::throwLocalException(const Ice::Current&)
{
    throw Ice::TimeoutException(__FILE__, __LINE__);
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
ThrowerI::throwMemoryLimitException(const Ice::ByteSeq&, const Ice::Current&)
{
    return Ice::ByteSeq(1024 * 20); // 20 KB.
}

void
ThrowerI::throwLocalExceptionIdempotent(const Ice::Current&)
{
    throw Ice::TimeoutException(__FILE__, __LINE__);
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
