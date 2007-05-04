// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestI.h>
#include <TestCommon.h>

MyDerivedClassI::MyDerivedClassI()
{
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
#ifdef __BCPLUSPLUS__
    return Test::MyDerivedClass::ice_isA(s, current);
#else
    return MyDerivedClass::ice_isA(s, current);
#endif
}
