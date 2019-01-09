// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace Test;

TestFacetI::TestFacetI()
{
}

Ice::PropertyDict
TestFacetI::getChanges(const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    return _changes;
}

void
TestFacetI::updated(const Ice::PropertyDict& changes)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _changes = changes;
}
