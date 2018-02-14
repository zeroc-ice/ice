// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ServantLocatorI.h>
#include <TestCommon.h>
#include <TestI.h>

using namespace std;
using namespace Ice;
using namespace Test;

ServantLocatorI::ServantLocatorI() :
    _deactivated(false)
{
}

ServantLocatorI::~ServantLocatorI()
{
    test(_deactivated);
}

Ice::ObjectPtr
ServantLocatorI::locate(const Ice::Current& current, Ice::LocalObjectPtr& cookie)
{
    test(!_deactivated);

    test(current.id.category == "");
    test(current.id.name == "test");

    cookie = new CookieI;

    return new TestI;
}

void
ServantLocatorI::finished(const Ice::Current&, const Ice::ObjectPtr&,
                          const Ice::LocalObjectPtr& cookie)
{
    test(!_deactivated);

    CookiePtr co = CookiePtr::dynamicCast(cookie);
    test(co);
    test(co->message() == "blahblah");
}

void
ServantLocatorI::deactivate(const string&)
{
    test(!_deactivated);

    _deactivated = true;
}
