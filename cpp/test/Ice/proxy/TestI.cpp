// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;

MyDerivedClassI::MyDerivedClassI()
{
}

#ifdef ICE_CPP11_MAPPING
shared_ptr<Ice::ObjectPrx>
MyDerivedClassI::echo(shared_ptr<Ice::ObjectPrx> obj, const Ice::Current&)
{
    return obj;
}
#else
Ice::ObjectPrx
MyDerivedClassI::echo(const Ice::ObjectPrx& obj, const Ice::Current&)
{
    return obj;
}
#endif

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

#ifdef ICE_CPP11_MAPPING
bool
MyDerivedClassI::ice_isA(string s, const Ice::Current& current) const
{
    _ctx = current.ctx;
    return Test::MyDerivedClass::ice_isA(move(s), current);
}
#else
bool
MyDerivedClassI::ice_isA(const std::string& s, const Ice::Current& current) const
{
    _ctx = current.ctx;
    return Test::MyDerivedClass::ice_isA(s, current);
}
#endif
