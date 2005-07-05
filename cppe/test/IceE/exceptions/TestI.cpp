// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestI.h>

using namespace Test;

ThrowerI::ThrowerI(const IceE::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
ThrowerI::shutdown(const IceE::Current&)
{
    _adapter->getCommunicator()->shutdown();
}

bool
ThrowerI::supportsUndeclaredExceptions(const IceE::Current&)
{
    return true;
}

bool
ThrowerI::supportsAssertException(const IceE::Current&)
{
    return false;
}

void
ThrowerI::throwAasA(IceE::Int a, const IceE::Current&)
{
    A ex;
    ex.aMem = a;
    throw ex;
}

void
ThrowerI::throwAorDasAorD(IceE::Int a, const IceE::Current&)
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
ThrowerI::throwBasA(IceE::Int a, IceE::Int b, const IceE::Current& current)
{
    throwBasB(a, b, current);
}

void
ThrowerI::throwCasA(IceE::Int a, IceE::Int b, IceE::Int c, const IceE::Current& current)
{
    throwCasC(a, b, c, current);
}

void
ThrowerI::throwBasB(IceE::Int a, IceE::Int b, const IceE::Current&)
{
    B ex;
    ex.aMem = a;
    ex.bMem = b;
    throw ex;
}

void
ThrowerI::throwCasB(IceE::Int a, IceE::Int b, IceE::Int c, const IceE::Current& current)
{
    throwCasC(a, b, c, current);
}

void
ThrowerI::throwCasC(IceE::Int a, IceE::Int b, IceE::Int c, const IceE::Current&)
{
    C ex;
    ex.aMem = a;
    ex.bMem = b;
    ex.cMem = c;
    throw ex;
}

void
ThrowerI::throwModA(IceE::Int a, IceE::Int a2, const IceE::Current&)
{
    Mod::A ex;
    ex.aMem = a;
    ex.a2Mem = a2;
    throw ex;
}

void
ThrowerI::throwUndeclaredA(IceE::Int a, const IceE::Current&)
{
    A ex;
    ex.aMem = a;
    throw ex;
}

void
ThrowerI::throwUndeclaredB(IceE::Int a, IceE::Int b, const IceE::Current&)
{
    B ex;
    ex.aMem = a;
    ex.bMem = b;
    throw ex;
}

void
ThrowerI::throwUndeclaredC(IceE::Int a, IceE::Int b, IceE::Int c, const IceE::Current&)
{
    C ex;
    ex.aMem = a;
    ex.bMem = b;
    ex.cMem = c;
    throw ex;
}

void
ThrowerI::throwLocalException(const IceE::Current&)
{
    throw IceE::TimeoutException(__FILE__, __LINE__);
}

void
ThrowerI::throwNonIceException(const IceE::Current&)
{
    throw int(12345);
}

void
ThrowerI::throwAssertException(const IceE::Current&)
{
    assert(false); // Not supported in C++.
}
