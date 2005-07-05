// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <IceE/IceE.h>

using namespace Test;

TestI::TestI(const IceE::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
TestI::baseAsBase(const ::IceE::Current&)
{
    Base b;
    b.b = "Base.b";
    throw b;
}

void
TestI::unknownDerivedAsBase(const ::IceE::Current&)
{
    UnknownDerived d;
    d.b = "UnknownDerived.b";
    d.ud = "UnknownDerived.ud";
    throw d;
}

void
TestI::knownDerivedAsBase(const ::IceE::Current&)
{
    KnownDerived d;
    d.b = "KnownDerived.b";
    d.kd = "KnownDerived.kd";
    throw d;
}

void
TestI::knownDerivedAsKnownDerived(const ::IceE::Current&)
{
    KnownDerived d;
    d.b = "KnownDerived.b";
    d.kd = "KnownDerived.kd";
    throw d;
}

void
TestI::unknownIntermediateAsBase(const ::IceE::Current&)
{
    UnknownIntermediate ui;
    ui.b = "UnknownIntermediate.b";
    ui.ui = "UnknownIntermediate.ui";
    throw ui;
}

void
TestI::knownIntermediateAsBase(const ::IceE::Current&)
{
    KnownIntermediate ki;
    ki.b = "KnownIntermediate.b";
    ki.ki = "KnownIntermediate.ki";
    throw ki;
}

void
TestI::knownMostDerivedAsBase(const ::IceE::Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    throw kmd;
}

void
TestI::knownIntermediateAsKnownIntermediate(const ::IceE::Current&)
{
    KnownIntermediate ki;
    ki.b = "KnownIntermediate.b";
    ki.ki = "KnownIntermediate.ki";
    throw ki;
}

void
TestI::knownMostDerivedAsKnownIntermediate(const ::IceE::Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    throw kmd;
}

void
TestI::
knownMostDerivedAsKnownMostDerived(const ::IceE::Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    throw kmd;
}

void
TestI::unknownMostDerived1AsBase(const ::IceE::Current&)
{
    UnknownMostDerived1 umd1;
    umd1.b = "UnknownMostDerived1.b";
    umd1.ki = "UnknownMostDerived1.ki";
    umd1.umd1 = "UnknownMostDerived1.umd1";
    throw umd1;
}

void
TestI::unknownMostDerived1AsKnownIntermediate(const ::IceE::Current&)
{
    UnknownMostDerived1 umd1;
    umd1.b = "UnknownMostDerived1.b";
    umd1.ki = "UnknownMostDerived1.ki";
    umd1.umd1 = "UnknownMostDerived1.umd1";
    throw umd1;
}

void
TestI::unknownMostDerived2AsBase(const ::IceE::Current&)
{
    UnknownMostDerived2 umd2;
    umd2.b = "UnknownMostDerived2.b";
    umd2.ui = "UnknownMostDerived2.ui";
    umd2.umd2 = "UnknownMostDerived2.umd2";
    throw umd2;
}

void
TestI::shutdown(const ::IceE::Current&)
{
    _adapter->getCommunicator()->shutdown();
}
