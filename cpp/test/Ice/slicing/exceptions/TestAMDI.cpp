// Copyright (c) ZeroC, Inc.

#include "TestAMDI.h"
#include "Ice/Ice.h"

using namespace Test;
using namespace std;
using namespace Ice;

TestI::TestI() = default;

void
TestI::baseAsBaseAsync(function<void()>, function<void(exception_ptr)> error, const Current&)
{
    try
    {
        throw Base{"Base.b"};
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
        throw UnknownDerived{"UnknownDerived.b", "UnknownDerived.ud"};
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
        throw KnownDerived{"KnownDerived.b", "KnownDerived.kd"};
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
        throw KnownDerived{"KnownDerived.b", "KnownDerived.kd"};
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
        throw UnknownIntermediate{"UnknownIntermediate.b", "UnknownIntermediate.ui"};
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
        throw KnownIntermediate{"KnownIntermediate.b", "KnownIntermediate.ki"};
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
        throw KnownMostDerived{"KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd"};
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
        throw KnownIntermediate{"KnownIntermediate.b", "KnownIntermediate.ki"};
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
        throw KnownMostDerived{"KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd"};
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
        throw KnownMostDerived{"KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd"};
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
        throw UnknownMostDerived1{"UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1"};
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
        throw UnknownMostDerived1{"UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1"};
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
        throw UnknownMostDerived2{"UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2"};
    }
    catch (...)
    {
        error(current_exception());
    }
}

void
TestI::shutdownAsync(function<void()> response, function<void(exception_ptr)>, const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}
