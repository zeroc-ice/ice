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
#ifdef ICE_CPP11_MAPPING
ServantLocatorI::locate(const Ice::Current& current, std::shared_ptr<void>& cookie)
#else
ServantLocatorI::locate(const Ice::Current& current, Ice::LocalObjectPtr& cookie)
#endif
{
    test(!_deactivated);

    test(current.id.category == "");
    test(current.id.name == "test");

    cookie = ICE_MAKE_SHARED(CookieI);

    return ICE_MAKE_SHARED(TestI);
}

void
#ifdef ICE_CPP11_MAPPING
ServantLocatorI::finished(const Ice::Current&, const Ice::ObjectPtr&, const std::shared_ptr<void>& cookie)
#else
ServantLocatorI::finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr& cookie)
#endif
{
    test(!_deactivated);
#ifdef ICE_CPP11_MAPPING
    shared_ptr<CookieI> co = static_pointer_cast<CookieI>(cookie);
#else
    CookiePtr co = CookiePtr::dynamicCast(cookie);
#endif
    test(co);
    test(co->message() == "blahblah");
}

void
ServantLocatorI::deactivate(const string&)
{
    test(!_deactivated);

    _deactivated = true;
}
