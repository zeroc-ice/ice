// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef TESTI_H
#define TESTI_H

#include <ServerPrivate.h>

class TestI : virtual public Test
{
public:

    TestI(const ::Ice::ObjectAdapterPtr&);
    virtual void baseAsBase(const ::Ice::Current&);
    virtual void unknownDerivedAsBase(const ::Ice::Current&);
    virtual void knownDerivedAsBase(const ::Ice::Current&);
    virtual void knownDerivedAsKnownDerived(const ::Ice::Current&);

    virtual void unknownIntermediateAsBase(const ::Ice::Current&);
    virtual void knownIntermediateAsBase(const ::Ice::Current&);
    virtual void knownMostDerivedAsBase(const ::Ice::Current&);
    virtual void knownIntermediateAsknownIntermediate(const ::Ice::Current&);
    virtual void knownMostDerivedAsKnownIntermediate(const ::Ice::Current&);
    virtual void knownMostDerivedAsKnownMostDerived(const ::Ice::Current&);

    virtual void unknownMostDerived1AsBase(const ::Ice::Current&);
    virtual void unknownMostDerived1AsKnownIntermediate(const ::Ice::Current&);
    virtual void unknownMostDerived2AsBase(const ::Ice::Current&);

    virtual void shutdown(const ::Ice::Current&);

private:

    const ::Ice::ObjectAdapterPtr& _adapter;
};

#endif
