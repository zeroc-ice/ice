// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class ThrowerI : public Thrower
{
public:

    ThrowerI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown();
    virtual void throwAasA(Ice::Int);
    virtual void throwAasAproxy();
    virtual void throwBasA(Ice::Int, Ice::Int);
    virtual void throwBasAproxy();
    virtual void throwCasA(Ice::Int, Ice::Int, Ice::Int);
    virtual void throwCasAproxy();
    virtual void throwBasB(Ice::Int, Ice::Int);
    virtual void throwBasBproxy();
    virtual void throwCasB(Ice::Int, Ice::Int, Ice::Int);
    virtual void throwCasBproxy();
    virtual void throwCasC(Ice::Int, Ice::Int, Ice::Int);
    virtual void throwCasCproxy();

private:

    Ice::ObjectAdapterPtr _adapter;
    APrx _a;
    BPrx _b;
    CPrx _c;
};

#endif
