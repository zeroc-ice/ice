// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestAMDI.h>
#include <TestHelper.h>

using namespace std;

MyDerivedClassI::MyDerivedClassI()
{
}

#ifdef ICE_CPP11_MAPPING
void
MyDerivedClassI::echoAsync(
    shared_ptr<Ice::ObjectPrx> obj,
    function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(obj);
}

void
MyDerivedClassI::shutdownAsync(
    function<void()> response,
    function<void(exception_ptr)>,
    const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}

void
MyDerivedClassI::getContextAsync(
    function<void(const Ice::Context&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(_ctx);
}
bool
MyDerivedClassI::ice_isA(string s, const Ice::Current& current) const
{
    _ctx = current.ctx;
    return Test::MyDerivedClass::ice_isA(move(s), current);
}
#else
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
MyDerivedClassI::ice_isA(const string& s, const Ice::Current& current) const
{
    _ctx = current.ctx;
    return Test::MyDerivedClass::ice_isA(s, current);
}

#endif
