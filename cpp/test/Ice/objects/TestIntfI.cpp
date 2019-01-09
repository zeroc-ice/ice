// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <Derived.h>
#include <DerivedEx.h>

using namespace Test;

BasePtr
TestIntfI::opDerived(const Ice::Current&)
{
    DerivedPtr d = ICE_MAKE_SHARED(Derived);
    d->theS.str = "S.str";
    d->str = "str";
    d->b = "b";
    return d;
}

void
TestIntfI::throwDerived(const Ice::Current&)
{
    DerivedEx ex;
    ex.reason = "reason";
    throw ex;
}
