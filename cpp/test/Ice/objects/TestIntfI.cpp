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
#include <Derived.h>
#include <DerivedEx.h>

using namespace Test;

BasePtr
TestIntfI::opDerived(const Ice::Current&)
{
    DerivedPtr d = new Derived();
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
