// **********************************************************************
//
// Copyright (c) 2001
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

#include <Test.h>

class ThrowerI : public Thrower
{
public:

    ThrowerI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);
    virtual void throwAasA(Ice::Int, const Ice::Current&);
    virtual void throwAorDasAorD(Ice::Int, const Ice::Current&);
    virtual void throwBasA(Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasA(Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwBasB(Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasB(Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasC(Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual bool supportsUndeclaredExceptions(const Ice::Current&);
    virtual void throwUndeclaredA(Ice::Int, const Ice::Current&);
    virtual void throwUndeclaredB(Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwUndeclaredC(Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwLocalException(const Ice::Current&);
    virtual void throwNonIceException(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
