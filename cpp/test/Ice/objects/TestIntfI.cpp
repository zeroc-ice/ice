// Copyright (c) ZeroC, Inc.

#include "Derived.h"
#include "DerivedEx.h"
#include "Ice/Ice.h"
#include "TestI.h"

using namespace Test;

BasePtr
TestIntfI::opDerived(const Ice::Current&)
{
    DerivedPtr d = std::make_shared<Derived>();
    d->theS.str = "S.str";
    d->str = "str";
    d->b = "b";
    return d;
}

void
TestIntfI::throwDerived(const Ice::Current&)
{
    throw DerivedEx{"reason"};
}
