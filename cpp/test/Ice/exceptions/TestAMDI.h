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

#ifndef TEST_I_H
#define TEST_I_H

#include <TestAMD.h>

class ThrowerI : public Thrower
{
public:

    ThrowerI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown_async(const AMD_Thrower_shutdownPtr&,
				const Ice::Current&);
    virtual void supportsUndeclaredExceptions_async(const AMD_Thrower_supportsUndeclaredExceptionsPtr&,
						    const Ice::Current&);
    virtual void supportsNonIceExceptions_async(const AMD_Thrower_supportsNonIceExceptionsPtr&,
						    const Ice::Current&);

    virtual void throwAasA_async(const AMD_Thrower_throwAasAPtr&,
				 Ice::Int, const Ice::Current&);
    virtual void throwAorDasAorD_async(const AMD_Thrower_throwAorDasAorDPtr&,
				       Ice::Int, const Ice::Current&);
    virtual void throwBasA_async(const AMD_Thrower_throwBasAPtr&,
				 Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasA_async(const AMD_Thrower_throwCasAPtr&,
				 Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwBasB_async(const AMD_Thrower_throwBasBPtr&,
				 Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasB_async(const AMD_Thrower_throwCasBPtr&,
				 Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasC_async(const AMD_Thrower_throwCasCPtr&,
				 Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwUndeclaredA_async(const AMD_Thrower_throwUndeclaredAPtr&,
					Ice::Int, const Ice::Current&);
    virtual void throwUndeclaredB_async(const AMD_Thrower_throwUndeclaredBPtr&,
					Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwUndeclaredC_async(const AMD_Thrower_throwUndeclaredCPtr&,
					Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwLocalException_async(const AMD_Thrower_throwLocalExceptionPtr&,
					   const Ice::Current&);
    virtual void throwNonIceException_async(const AMD_Thrower_throwNonIceExceptionPtr&,
					    const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
