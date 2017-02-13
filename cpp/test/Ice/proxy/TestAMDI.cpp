// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestAMDI.h>
#include <TestCommon.h>

MyDerivedClassI::MyDerivedClassI()
{
}

void
MyDerivedClassI::echo_async(const Test::AMD_MyDerivedClass_echoPtr& cb, const Ice::ObjectPrx& obj, const Ice::Current&)
{
    cb->ice_response(obj);
}

void
MyDerivedClassI::shutdown_async(const Test::AMD_MyClass_shutdownPtr& cb, const Ice::Current& c)
{
    c.adapter->getCommunicator()->shutdown();
    cb->ice_response();
}

void
MyDerivedClassI::getContext_async(const Test::AMD_MyClass_getContextPtr& cb, const Ice::Current&)
{
    cb->ice_response(_ctx);
}

bool
MyDerivedClassI::ice_isA(const std::string& s, const Ice::Current& current) const
{
    _ctx = current.ctx;
    return Test::MyDerivedClass::ice_isA(s, current);
}
