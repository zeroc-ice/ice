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

#include <TestAMDI.h>
#include <Ice/Ice.h>

TestI::TestI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
TestI::baseAsBase_async(const ::AMD_Test_baseAsBasePtr& cb, const ::Ice::Current&)
{
    Base b;
    b.b = "Base.b";
    cb->ice_exception(b);
}

void
TestI::unknownDerivedAsBase_async(const ::AMD_Test_unknownDerivedAsBasePtr& cb, const ::Ice::Current&)
{
    UnknownDerived d;
    d.b = "UnknownDerived.b";
    d.ud = "UnknownDerived.ud";
    cb->ice_exception(d);
}

void
TestI::knownDerivedAsBase_async(const ::AMD_Test_knownDerivedAsBasePtr& cb, const ::Ice::Current&)
{
    KnownDerived d;
    d.b = "KnownDerived.b";
    d.kd = "KnownDerived.kd";
    cb->ice_exception(d);
}

void
TestI::knownDerivedAsKnownDerived_async(const ::AMD_Test_knownDerivedAsKnownDerivedPtr& cb, const ::Ice::Current&)
{
    KnownDerived d;
    d.b = "KnownDerived.b";
    d.kd = "KnownDerived.kd";
    cb->ice_exception(d);
}

void
TestI::unknownIntermediateAsBase_async(const ::AMD_Test_unknownIntermediateAsBasePtr& cb, const ::Ice::Current&)
{
    UnknownIntermediate ui;
    ui.b = "UnknownIntermediate.b";
    ui.ui = "UnknownIntermediate.ui";
    cb->ice_exception(ui);
}

void
TestI::knownIntermediateAsBase_async(const ::AMD_Test_knownIntermediateAsBasePtr& cb, const ::Ice::Current&)
{
    KnownIntermediate ki;
    ki.b = "KnownIntermediate.b";
    ki.ki = "KnownIntermediate.ki";
    cb->ice_exception(ki);
}

void
TestI::knownMostDerivedAsBase_async(const ::AMD_Test_knownMostDerivedAsBasePtr& cb, const ::Ice::Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    cb->ice_exception(kmd);
}

void
TestI::knownIntermediateAsKnownIntermediate_async(const ::AMD_Test_knownIntermediateAsKnownIntermediatePtr& cb,
	                                          const ::Ice::Current&)
{
    KnownIntermediate ki;
    ki.b = "KnownIntermediate.b";
    ki.ki = "KnownIntermediate.ki";
    cb->ice_exception(ki);
}

void
TestI::knownMostDerivedAsKnownIntermediate_async(const ::AMD_Test_knownMostDerivedAsKnownIntermediatePtr& cb,
	                                         const ::Ice::Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    cb->ice_exception(kmd);
}

void
TestI::
knownMostDerivedAsKnownMostDerived_async(const ::AMD_Test_knownMostDerivedAsKnownMostDerivedPtr& cb,
	                                 const ::Ice::Current&)
{
    KnownMostDerived kmd;
    kmd.b = "KnownMostDerived.b";
    kmd.ki = "KnownMostDerived.ki";
    kmd.kmd = "KnownMostDerived.kmd";
    cb->ice_exception(kmd);
}

void
TestI::unknownMostDerived1AsBase_async(const ::AMD_Test_unknownMostDerived1AsBasePtr& cb, const ::Ice::Current&)
{
    UnknownMostDerived1 umd1;
    umd1.b = "UnknownMostDerived1.b";
    umd1.ki = "UnknownMostDerived1.ki";
    umd1.umd1 = "UnknownMostDerived1.umd1";
    cb->ice_exception(umd1);
}

void
TestI::unknownMostDerived1AsKnownIntermediate_async(const ::AMD_Test_unknownMostDerived1AsKnownIntermediatePtr& cb,
	                                            const ::Ice::Current&)
{
    UnknownMostDerived1 umd1;
    umd1.b = "UnknownMostDerived1.b";
    umd1.ki = "UnknownMostDerived1.ki";
    umd1.umd1 = "UnknownMostDerived1.umd1";
    cb->ice_exception(umd1);
}

void
TestI::unknownMostDerived2AsBase_async(const ::AMD_Test_unknownMostDerived2AsBasePtr& cb, const ::Ice::Current&)
{
    UnknownMostDerived2 umd2;
    umd2.b = "UnknownMostDerived2.b";
    umd2.ui = "UnknownMostDerived2.ui";
    umd2.umd2 = "UnknownMostDerived2.umd2";
    cb->ice_exception(umd2);
}

void
TestI::shutdown_async(const ::AMD_Test_shutdownPtr& cb, const ::Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
    cb->ice_response();
}
