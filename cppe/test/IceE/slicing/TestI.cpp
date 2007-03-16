// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <TestCommon.h>
#include <IceE/IceE.h>

using namespace Test;

TestI::TestI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
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
TestI::shutdown(const ::Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
#ifdef _WIN32_WCE
    tprintf("The server has shutdown, close the window to terminate the server.");
#endif
}
