// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <TestI.h>

using namespace Test;

ThrowerI::ThrowerI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
ThrowerI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
#ifdef _WIN32_WCE
    tprintf("The server has shutdown, close the window to terminate the server.");
#endif
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
