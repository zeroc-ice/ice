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

#include <ServerTest.h>
#include <Forward.h>

class TestI : virtual public Test
{
public:

    TestI(const ::Ice::ObjectAdapterPtr&);

    virtual ::Ice::ObjectPtr SBaseAsObject(const ::Ice::Current&);
    virtual SBasePtr SBaseAsSBase(const ::Ice::Current&);
    virtual SBasePtr SBSKnownDerivedAsSBase(const ::Ice::Current&);
    virtual SBSKnownDerivedPtr SBSKnownDerivedAsSBSKnownDerived(const ::Ice::Current&);

    virtual SBasePtr SBSUnknownDerivedAsSBase(const ::Ice::Current&);

    virtual ::Ice::ObjectPtr SUnknownAsObject(const ::Ice::Current&);

    virtual BPtr oneElementCycle(const ::Ice::Current&);
    virtual BPtr twoElementCycle(const ::Ice::Current&);

    virtual BPtr D1AsB(const ::Ice::Current&);
    virtual D1Ptr D1AsD1(const ::Ice::Current&);
    virtual BPtr D2AsB(const ::Ice::Current&);

    virtual void paramTest1(BPtr&, BPtr&, const ::Ice::Current&);
    virtual void paramTest2(BPtr&, BPtr&, const ::Ice::Current&);
    virtual BPtr paramTest3(BPtr&, BPtr&, const ::Ice::Current&);
    virtual BPtr paramTest4(BPtr&, const ::Ice::Current&);

    virtual BPtr returnTest1(BPtr&, BPtr&, const ::Ice::Current&);
    virtual BPtr returnTest2(BPtr&, BPtr&, const ::Ice::Current&);
    virtual BPtr returnTest3(const BPtr&, const BPtr&, const ::Ice::Current&);

    virtual SS sequenceTest(const SS1Ptr&, const SS2Ptr&, const ::Ice::Current&);

    virtual void throwBaseAsBase(const ::Ice::Current&);
    virtual void throwDerivedAsBase(const ::Ice::Current&);
    virtual void throwDerivedAsDerived(const ::Ice::Current&);
    virtual void throwUnknownDerivedAsBase(const ::Ice::Current&);

    virtual void useForward(::ForwardPtr&, const ::Ice::Current&);

    virtual void shutdown(const ::Ice::Current&);

private:

    const ::Ice::ObjectAdapterPtr& _adapter;
};

#endif
