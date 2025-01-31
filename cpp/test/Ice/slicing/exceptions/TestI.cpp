// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace std;
using namespace Test;
using namespace Ice;

TestI::TestI() = default;

void
TestI::baseAsBase(const Current&)
{
    throw Base{"Base.b"};
}

void
TestI::unknownDerivedAsBase(const Current&)
{
    throw UnknownDerived{"UnknownDerived.b", "UnknownDerived.ud"};
}

void
TestI::knownDerivedAsBase(const Current&)
{
    throw KnownDerived{"KnownDerived.b", "KnownDerived.kd"};
}

void
TestI::knownDerivedAsKnownDerived(const Current&)
{
    throw KnownDerived{"KnownDerived.b", "KnownDerived.kd"};
}

void
TestI::unknownIntermediateAsBase(const Current&)
{
    throw UnknownIntermediate{"UnknownIntermediate.b", "UnknownIntermediate.ui"};
}

void
TestI::knownIntermediateAsBase(const Current&)
{
    throw KnownIntermediate{"KnownIntermediate.b", "KnownIntermediate.ki"};
}

void
TestI::knownMostDerivedAsBase(const Current&)
{
    throw KnownMostDerived{"KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd"};
}

void
TestI::knownIntermediateAsKnownIntermediate(const Current&)
{
    throw KnownIntermediate{"KnownIntermediate.b", "KnownIntermediate.ki"};
}

void
TestI::knownMostDerivedAsKnownIntermediate(const Current&)
{
    throw KnownMostDerived{"KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd"};
}

void
TestI::knownMostDerivedAsKnownMostDerived(const Current&)
{
    throw KnownMostDerived{"KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd"};
}

void
TestI::unknownMostDerived1AsBase(const Current&)
{
    throw UnknownMostDerived1{"UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1"};
}

void
TestI::unknownMostDerived1AsKnownIntermediate(const Current&)
{
    throw UnknownMostDerived1{"UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1"};
}

void
TestI::unknownMostDerived2AsBase(const Current&)
{
    throw UnknownMostDerived2{"UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2"};
}

void
TestI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
