// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace std;
using namespace Ice;

MyDerivedInterfaceI::MyDerivedInterfaceI() = default;

optional<ObjectPrx>
MyDerivedInterfaceI::echo(optional<ObjectPrx> obj, const Current&)
{
    return obj;
}

void
MyDerivedInterfaceI::shutdown(const Current& c)
{
    c.adapter->getCommunicator()->shutdown();
}

Context
MyDerivedInterfaceI::getContext(const Current&)
{
    return _ctx;
}

bool
MyDerivedInterfaceI::ice_isA(string s, const Current& current) const
{
    _ctx = current.ctx;
    return Test::MyDerivedInterface::ice_isA(std::move(s), current);
}
