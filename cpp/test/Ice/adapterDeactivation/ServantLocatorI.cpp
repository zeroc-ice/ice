// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <ServantLocatorI.h>
#include <TestCommon.h>
#include <TestI.h>

using namespace std;
using namespace Ice;

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
ServantLocatorI::finished(const Ice::Current& current, const Ice::ObjectPtr& servant,
			  const Ice::LocalObjectPtr& cookie)
{
    test(!_deactivated);

    CookiePtr co = CookiePtr::dynamicCast(cookie);
    test(co);
    test(co->message() == "blahblah");
}

void
ServantLocatorI::deactivate()
{
    test(!_deactivated);

    _deactivated = true;
}
