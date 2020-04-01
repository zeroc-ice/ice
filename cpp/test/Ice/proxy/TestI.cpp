//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;

MyDerivedClassI::MyDerivedClassI()
{
}

shared_ptr<Ice::ObjectPrx>
MyDerivedClassI::echo(shared_ptr<Ice::ObjectPrx> obj, const Ice::Current&)
{
    return obj;
}

void
MyDerivedClassI::shutdown(const Ice::Current& c)
{
    c.adapter->getCommunicator()->shutdown();
}

Ice::Context
MyDerivedClassI::getContext(const Ice::Current&)
{
    return _ctx;
}

bool
MyDerivedClassI::ice_isA(string s, const Ice::Current& current) const
{
    _ctx = current.ctx;
    return Test::MyDerivedClass::ice_isA(move(s), current);
}
