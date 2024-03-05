//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <TestI.h>
#include <Ice/Ice.h>
#include <TestHelper.h>

using namespace std;
using namespace Test;
using namespace Ice;

TestI::TestI()
{
}

void
TestI::baseAsBase(const Current&)
{
    Base b;
    b.b = "Base.b";
    throw b;
}

void
TestI::unknownDerivedAsBase(const Current&)
{
    UnknownDerived d;
    d.b = "UnknownDerived.b";
    d.ud = "UnknownDerived.ud";
    throw d;
}

void
TestI::knownDerivedAsBase(const Current&)
{
    KnownDerived d;
    d.b = "KnownDerived.b";
    d.kd = "KnownDerived.kd";
    throw d;
}

void
TestI::knownDerivedAsKnownDerived(const Current&)
{
    KnownDerived d;
    d.b = "KnownDerived.b";
    d.kd = "KnownDerived.kd";
    throw d;
}

void
TestI::unknownIntermediateAsBase(const Current&)
{
    UnknownIntermediate ui;
    ui.b = "UnknownIntermediate.b";
    ui.ui = "UnknownIntermediate.ui";
    throw ui;
}

void
TestI::knownIntermediateAsBase(const Current&)
{
    KnownIntermediate ki;
    ki.b = "KnownIntermediate.b";
    ki.ki = "KnownIntermediate.ki";
    throw ki;
}

void
TestI::knownMostDerivedAsBase(const Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    throw kmd;
}

void
TestI::knownIntermediateAsKnownIntermediate(const Current&)
{
    KnownIntermediate ki;
    ki.b = "KnownIntermediate.b";
    ki.ki = "KnownIntermediate.ki";
    throw ki;
}

void
TestI::knownMostDerivedAsKnownIntermediate(const Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    throw kmd;
}

void
TestI::
knownMostDerivedAsKnownMostDerived(const Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    throw kmd;
}

void
TestI::unknownMostDerived1AsBase(const Current&)
{
    UnknownMostDerived1 umd1;
    umd1.b = "UnknownMostDerived1.b";
    umd1.ki = "UnknownMostDerived1.ki";
    umd1.umd1 = "UnknownMostDerived1.umd1";
    throw umd1;
}

void
TestI::unknownMostDerived1AsKnownIntermediate(const Current&)
{
    UnknownMostDerived1 umd1;
    umd1.b = "UnknownMostDerived1.b";
    umd1.ki = "UnknownMostDerived1.ki";
    umd1.umd1 = "UnknownMostDerived1.umd1";
    throw umd1;
}

void
TestI::unknownMostDerived2AsBase(const Current&)
{
    UnknownMostDerived2 umd2;
    umd2.b = "UnknownMostDerived2.b";
    umd2.ui = "UnknownMostDerived2.ui";
    umd2.umd2 = "UnknownMostDerived2.umd2";
    throw umd2;
}

void
TestI::unknownMostDerived2AsBaseCompact(const Current&)
{
    UnknownMostDerived2 umd2;
    umd2.b = "UnknownMostDerived2.b";
    umd2.ui = "UnknownMostDerived2.ui";
    umd2.umd2 = "UnknownMostDerived2.umd2";
    throw umd2;
}

void
TestI::knownPreservedAsBase(const Current&)
{
    KnownPreservedDerived ex;
    ex.b = "base";
    ex.kp = "preserved";
    ex.kpd = "derived";
    throw ex;
}

void
TestI::knownPreservedAsKnownPreserved(const Current&)
{
    KnownPreservedDerived ex;
    ex.b = "base";
    ex.kp = "preserved";
    ex.kpd = "derived";
    throw ex;
}

void
TestI::relayKnownPreservedAsBase(optional<RelayPrx> r, const Current& c)
{
    RelayPrx p(c.con->createProxy(r->ice_getIdentity()));
    p->knownPreservedAsBase();
    test(false);
}

void
TestI::relayKnownPreservedAsKnownPreserved(optional<RelayPrx> r, const Current& c)
{
    RelayPrx p(c.con->createProxy(r->ice_getIdentity()));
    p->knownPreservedAsKnownPreserved();
    test(false);
}

void
TestI::unknownPreservedAsBase(const Current&)
{
    SPreserved2 ex;
    ex.b = "base";
    ex.kp = "preserved";
    ex.kpd = "derived";
    ex.p1 = make_shared<SPreservedClass>("bc", "spc");
    ex.p2 = ex.p1;
    throw ex;
}

void
TestI::unknownPreservedAsKnownPreserved(const Current&)
{
    SPreserved2 ex;
    ex.b = "base";
    ex.kp = "preserved";
    ex.kpd = "derived";
    ex.p1 = make_shared<SPreservedClass>("bc", "spc");
    ex.p2 = ex.p1;
    throw ex;
}

void
TestI::relayUnknownPreservedAsBase(optional<RelayPrx> r, const Current& c)
{
    RelayPrx p = r->ice_fixed(c.con);
    p->unknownPreservedAsBase();
    test(false);
}

void
TestI::relayUnknownPreservedAsKnownPreserved(optional<RelayPrx> r, const Current& c)
{
    RelayPrx p = r->ice_fixed(c.con);
    p->unknownPreservedAsKnownPreserved();
    test(false);
}

void
TestI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
