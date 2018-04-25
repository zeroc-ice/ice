// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <Ice/Ice.h>
#include <TestCommon.h>

using namespace Test;

TestI::TestI()
{
}

void
TestI::baseAsBase(const ::Ice::Current&)
{
    Base b;
    b.b = "Base.b";
    throw b;
}

void
TestI::unknownDerivedAsBase(const ::Ice::Current&)
{
    UnknownDerived d;
    d.b = "UnknownDerived.b";
    d.ud = "UnknownDerived.ud";
    throw d;
}

void
TestI::knownDerivedAsBase(const ::Ice::Current&)
{
    KnownDerived d;
    d.b = "KnownDerived.b";
    d.kd = "KnownDerived.kd";
    throw d;
}

void
TestI::knownDerivedAsKnownDerived(const ::Ice::Current&)
{
    KnownDerived d;
    d.b = "KnownDerived.b";
    d.kd = "KnownDerived.kd";
    throw d;
}

void
TestI::unknownIntermediateAsBase(const ::Ice::Current&)
{
    UnknownIntermediate ui;
    ui.b = "UnknownIntermediate.b";
    ui.ui = "UnknownIntermediate.ui";
    throw ui;
}

void
TestI::knownIntermediateAsBase(const ::Ice::Current&)
{
    KnownIntermediate ki;
    ki.b = "KnownIntermediate.b";
    ki.ki = "KnownIntermediate.ki";
    throw ki;
}

void
TestI::knownMostDerivedAsBase(const ::Ice::Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    throw kmd;
}

void
TestI::knownIntermediateAsKnownIntermediate(const ::Ice::Current&)
{
    KnownIntermediate ki;
    ki.b = "KnownIntermediate.b";
    ki.ki = "KnownIntermediate.ki";
    throw ki;
}

void
TestI::knownMostDerivedAsKnownIntermediate(const ::Ice::Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    throw kmd;
}

void
TestI::
knownMostDerivedAsKnownMostDerived(const ::Ice::Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    throw kmd;
}

void
TestI::unknownMostDerived1AsBase(const ::Ice::Current&)
{
    UnknownMostDerived1 umd1;
    umd1.b = "UnknownMostDerived1.b";
    umd1.ki = "UnknownMostDerived1.ki";
    umd1.umd1 = "UnknownMostDerived1.umd1";
    throw umd1;
}

void
TestI::unknownMostDerived1AsKnownIntermediate(const ::Ice::Current&)
{
    UnknownMostDerived1 umd1;
    umd1.b = "UnknownMostDerived1.b";
    umd1.ki = "UnknownMostDerived1.ki";
    umd1.umd1 = "UnknownMostDerived1.umd1";
    throw umd1;
}

void
TestI::unknownMostDerived2AsBase(const ::Ice::Current&)
{
    UnknownMostDerived2 umd2;
    umd2.b = "UnknownMostDerived2.b";
    umd2.ui = "UnknownMostDerived2.ui";
    umd2.umd2 = "UnknownMostDerived2.umd2";
    throw umd2;
}

void
TestI::unknownMostDerived2AsBaseCompact(const ::Ice::Current&)
{
    UnknownMostDerived2 umd2;
    umd2.b = "UnknownMostDerived2.b";
    umd2.ui = "UnknownMostDerived2.ui";
    umd2.umd2 = "UnknownMostDerived2.umd2";
    throw umd2;
}

void
TestI::knownPreservedAsBase(const ::Ice::Current&)
{
    KnownPreservedDerived ex;
    ex.b = "base";
    ex.kp = "preserved";
    ex.kpd = "derived";
    throw ex;
}

void
TestI::knownPreservedAsKnownPreserved(const ::Ice::Current&)
{
    KnownPreservedDerived ex;
    ex.b = "base";
    ex.kp = "preserved";
    ex.kpd = "derived";
    throw ex;
}

void
TestI::relayKnownPreservedAsBase(ICE_IN(RelayPrxPtr) r, const ::Ice::Current& c)
{
    RelayPrxPtr p = ICE_UNCHECKED_CAST(RelayPrx, c.con->createProxy(r->ice_getIdentity()));
    p->knownPreservedAsBase();
    test(false);
}

void
TestI::relayKnownPreservedAsKnownPreserved(ICE_IN(RelayPrxPtr) r, const ::Ice::Current& c)
{
    RelayPrxPtr p = ICE_UNCHECKED_CAST(RelayPrx, c.con->createProxy(r->ice_getIdentity()));
    p->knownPreservedAsKnownPreserved();
    test(false);
}

void
TestI::unknownPreservedAsBase(const ::Ice::Current&)
{
    SPreserved2 ex;
    ex.b = "base";
    ex.kp = "preserved";
    ex.kpd = "derived";
    ex.p1 = ICE_MAKE_SHARED(SPreservedClass, "bc", "spc");
    ex.p2 = ex.p1;
    throw ex;
}

void
TestI::unknownPreservedAsKnownPreserved(const ::Ice::Current&)
{
    SPreserved2 ex;
    ex.b = "base";
    ex.kp = "preserved";
    ex.kpd = "derived";
    ex.p1 = ICE_MAKE_SHARED(SPreservedClass, "bc", "spc");
    ex.p2 = ex.p1;
    throw ex;
}

void
TestI::relayUnknownPreservedAsBase(ICE_IN(RelayPrxPtr) r, const ::Ice::Current& c)
{
    RelayPrxPtr p = ICE_UNCHECKED_CAST(RelayPrx, c.con->createProxy(r->ice_getIdentity()));
    p->unknownPreservedAsBase();
    test(false);
}

void
TestI::relayUnknownPreservedAsKnownPreserved(ICE_IN(RelayPrxPtr) r, const ::Ice::Current& c)
{
    RelayPrxPtr p = ICE_UNCHECKED_CAST(RelayPrx, c.con->createProxy(r->ice_getIdentity()));
    p->unknownPreservedAsKnownPreserved();
    test(false);
}

void
TestI::shutdown(const ::Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
