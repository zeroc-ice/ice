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
#include <TestCommon.h>

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
    return Test::MyDerivedClassDisp::ice_isA(move(s), current);
}
#else
bool
MyDerivedClassI::ice_isA(const std::string& s, const Ice::Current& current) const
{
    _ctx = current.ctx;
    return Test::MyDerivedClass::ice_isA(s, current);
}
#endif
