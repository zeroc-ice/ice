// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace Test;
using namespace std;

TestFacetI::TestFacetI() = default;

Ice::PropertyDict
TestFacetI::getChanges(const Ice::Current&)
{
    lock_guard lock(_mutex);
    return _changes;
}

void
TestFacetI::updated(const Ice::PropertyDict& changes)
{
    lock_guard lock(_mutex);
    _changes = changes;
}
