// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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

Ice::ObjectPrx
MyDerivedClassI::echo(const Ice::ObjectPrx& obj, const Ice::Current&)
{
    return obj;
}

void
MyDerivedClassI::shutdown(const Ice::Current& c)
{
    c.adapter->getCommunicator()->shutdown();
}

Ice::Context
MyDerivedClassI::getContext(const Ice::Current& c)
{
    return _ctx;
}

bool
MyDerivedClassI::ice_isA(const std::string& s, const Ice::Current& current) const
{
    _ctx = current.ctx;
    
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    return MyDerivedClass::ice_isA(s, current);
#else
    return Test::MyDerivedClass::ice_isA(s, current);
#endif
}
