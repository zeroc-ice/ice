// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
