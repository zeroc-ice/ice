// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TESTI_H
#define TESTI_H

#include <ServerPrivate.h>

class TestI : virtual public Test::TestIntf
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
    virtual void knownIntermediateAsKnownIntermediate(const ::Ice::Current&);
    virtual void knownMostDerivedAsKnownIntermediate(const ::Ice::Current&);
    virtual void knownMostDerivedAsKnownMostDerived(const ::Ice::Current&);

    virtual void unknownMostDerived1AsBase(const ::Ice::Current&);
    virtual void unknownMostDerived1AsKnownIntermediate(const ::Ice::Current&);
    virtual void unknownMostDerived2AsBase(const ::Ice::Current&);

    virtual void shutdown(const ::Ice::Current&);

private:

    const ::Ice::ObjectAdapterPtr _adapter;
};

#endif
