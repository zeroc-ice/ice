// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestAMDI.h>

ThrowerI::ThrowerI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
ThrowerI::shutdown_async(const AMD_Thrower_shutdownPtr& cb,
			 const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
    cb->ice_response();
}

void
ThrowerI::supportsUndeclaredExceptions_async(const AMD_Thrower_supportsUndeclaredExceptionsPtr& cb,
					     const Ice::Current&)
{
    cb->ice_response(true);
}

void
ThrowerI::throwAasA_async(const AMD_Thrower_throwAasAPtr& cb,
			  Ice::Int a, const Ice::Current&)
{
    A ex;
    ex.aMem = a;
    cb->ice_exception(ex);
}

void
ThrowerI::throwAorDasAorD_async(const AMD_Thrower_throwAorDasAorDPtr& cb,
				Ice::Int a, const Ice::Current&)
{
    if(a > 0)
    {
	A ex;
	ex.aMem = a;
	cb->ice_exception(ex);
    }
    else
    {
	D ex;
	ex.dMem = a;
	cb->ice_exception(ex);
    }
}

void
ThrowerI::throwBasA_async(const AMD_Thrower_throwBasAPtr& cb,
			  Ice::Int a, Ice::Int b, const Ice::Current& current)
{
    B ex;
    ex.aMem = a;
    ex.bMem = b;
    throw ex;
    //cb->ice_exception(ex);
}

void
ThrowerI::throwCasA_async(const AMD_Thrower_throwCasAPtr& cb,
			  Ice::Int a, Ice::Int b, Ice::Int c, const Ice::Current& current)
{
    C ex;
    ex.aMem = a;
    ex.bMem = b;
    ex.cMem = c;
    cb->ice_exception(ex);
}

void
ThrowerI::throwBasB_async(const AMD_Thrower_throwBasBPtr& cb,
			  Ice::Int a, Ice::Int b, const Ice::Current&)
{
    B ex;
    ex.aMem = a;
    ex.bMem = b;
    throw ex;
    //cb->ice_exception(ex);
}

void
ThrowerI::throwCasB_async(const AMD_Thrower_throwCasBPtr& cb,
			  Ice::Int a, Ice::Int b, Ice::Int c, const Ice::Current& current)
{
    C ex;
    ex.aMem = a;
    ex.bMem = b;
    ex.cMem = c;
    cb->ice_exception(ex);
}

void
ThrowerI::throwCasC_async(const AMD_Thrower_throwCasCPtr& cb,
			  Ice::Int a, Ice::Int b, Ice::Int c, const Ice::Current&)
{
    C ex;
    ex.aMem = a;
    ex.bMem = b;
    ex.cMem = c;
    cb->ice_exception(ex);
}

void
ThrowerI::throwUndeclaredA_async(const AMD_Thrower_throwUndeclaredAPtr& cb,
				 Ice::Int a, const Ice::Current&)
{
    A ex;
    ex.aMem = a;
    cb->ice_exception(ex);
}

void
ThrowerI::throwUndeclaredB_async(const AMD_Thrower_throwUndeclaredBPtr& cb,
				 Ice::Int a, Ice::Int b, const Ice::Current&)
{
    B ex;
    ex.aMem = a;
    ex.bMem = b;
    throw ex;
//    cb->ice_exception(ex);
}

void
ThrowerI::throwUndeclaredC_async(const AMD_Thrower_throwUndeclaredCPtr& cb,
				 Ice::Int a, Ice::Int b, Ice::Int c, const Ice::Current&)
{
    C ex;
    ex.aMem = a;
    ex.bMem = b;
    ex.cMem = c;
    cb->ice_exception(ex);
}

void
ThrowerI::throwLocalException_async(const AMD_Thrower_throwLocalExceptionPtr& cb,
				    const Ice::Current&)
{
    cb->ice_exception(Ice::TimeoutException(__FILE__, __LINE__));
}

void
ThrowerI::throwNonIceException_async(const AMD_Thrower_throwNonIceExceptionPtr&,
				     const Ice::Current&)
{
    throw int(12345);
}
