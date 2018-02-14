// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace Test;

TestFacetI::TestFacetI() :
    _called(false)
{
}

Ice::PropertyDict
TestFacetI::getChanges(const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // The client calls PropertiesAdmin::setProperties() and then invokes
    // this operation. Since setProperties() is implemented using AMD, the
    // client might receive its reply and then call getChanges() before our
    // updated() method is called. We block here to ensure that updated()
    // gets called before we return the most recent set of changes.
    //
    while(!_called)
    {
        wait();
    }

    _called = false;

    return _changes;
}

void
TestFacetI::updated(const Ice::PropertyDict& changes)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    _changes = changes;
    _called = true;
    notify();
}
