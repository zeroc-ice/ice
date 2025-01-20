// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace std;
using namespace Ice;

MyDerivedClassI::MyDerivedClassI() = default;

optional<ObjectPrx>
MyDerivedClassI::echo(optional<ObjectPrx> obj, const Current&)
{
    return obj;
}

void
MyDerivedClassI::shutdown(const Current& c)
{
    c.adapter->getCommunicator()->shutdown();
}

Context
MyDerivedClassI::getContext(const Current&)
{
    return _ctx;
}

bool
MyDerivedClassI::ice_isA(string s, const Current& current) const
{
    _ctx = current.ctx;
    return Test::MyDerivedClass::ice_isA(std::move(s), current);
}
