//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include <TestI.h>

using namespace Test;
using namespace std;

TestFacetI::TestFacetI() {}

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
