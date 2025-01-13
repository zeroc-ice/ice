//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestAMDI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace Test;
using namespace std;
using namespace Ice;

TestI::TestI() = default;

void
TestI::baseAsBaseAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        Base b;
        b.b = "Base.b";
        throw b;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::unknownDerivedAsBaseAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        UnknownDerived d;
        d.b = "UnknownDerived.b";
        d.ud = "UnknownDerived.ud";
        throw d;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::knownDerivedAsBaseAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        KnownDerived d;
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::knownDerivedAsKnownDerivedAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        KnownDerived d;
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::unknownIntermediateAsBaseAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        UnknownIntermediate ui;
        ui.b = "UnknownIntermediate.b";
        ui.ui = "UnknownIntermediate.ui";
        throw ui;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::knownIntermediateAsBaseAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        KnownIntermediate ki;
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::knownMostDerivedAsBaseAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        KnownMostDerived kmd;
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::knownIntermediateAsKnownIntermediateAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        KnownIntermediate ki;
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::knownMostDerivedAsKnownIntermediateAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        KnownMostDerived kmd;
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::knownMostDerivedAsKnownMostDerivedAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        KnownMostDerived kmd;
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::unknownMostDerived1AsBaseAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        UnknownMostDerived1 umd1;
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::unknownMostDerived1AsKnownIntermediateAsync(
    function<void()>,
    function<void(exception_ptr)> error,
    const Current&)
{
    try
    {
        UnknownMostDerived1 umd1;
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::unknownMostDerived2AsBaseAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        UnknownMostDerived2 umd2;
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        throw umd2;
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::shutdownAsync(function<void()> response, function<void(exception_ptr)>, const ::Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}
