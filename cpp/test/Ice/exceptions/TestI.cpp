// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

ThrowerI::ThrowerI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
ThrowerI::shutdown()
{
    _adapter->getCommunicator()->shutdown();
}

void
ThrowerI::throwAasA(Ice::Int a)
{
    A ex;
    ex.a = a;
    throw ex;
}

void
ThrowerI::throwBasA(Ice::Int a, Ice::Int b)
{
    throwBasB(a, b);
}

void
ThrowerI::throwCasA(Ice::Int a, Ice::Int b, Ice::Int c)
{
    throwCasC(a, b, c);
}

void
ThrowerI::throwBasB(Ice::Int a, Ice::Int b)
{
    B ex;
    ex.a = a;
    ex.b = b;
    throw ex;
}

void
ThrowerI::throwCasB(Ice::Int a, Ice::Int b, Ice::Int c)
{
    throwCasC(a, b, c);
}

void
ThrowerI::throwCasC(Ice::Int a, Ice::Int b, Ice::Int c)
{
    C ex;
    ex.a = a;
    ex.b = b;
    ex.c = c;
    throw ex;
}
