// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ServantLocatorI.h>
#include <TestHelper.h>
#include <Test.h>
#include <TestI.h>

using namespace std;
using namespace Ice;

class ServantLocatorI : public Test::ServantLocatorI
{
public:

    ServantLocatorI(const string& category) : Test::ServantLocatorI(category)
    {
    }

protected:

#ifdef ICE_CPP11_MAPPING
    virtual Ice::ObjectPtr
    newServantAndCookie(shared_ptr<void>& cookie) const
    {
        cookie = make_shared<CookieI>();
        return make_shared<TestI>();
    }

    virtual void
    checkCookie(const shared_ptr<void>& cookie) const
    {
        Test::CookiePtr co = static_pointer_cast<Test::Cookie>(cookie);
        test(co);
        test(co->message() == "blahblah");
    }
#else
    virtual Ice::ObjectPtr
    newServantAndCookie(Ice::LocalObjectPtr& cookie) const
    {
        cookie = new CookieI();
        return new TestI();
    }

    virtual void
    checkCookie(const Ice::LocalObjectPtr& cookie) const
    {
        Test::CookiePtr co = Test::CookiePtr::dynamicCast(cookie);
        test(co);
        test(co->message() == "blahblah");
    }
#endif
    virtual void
    throwTestIntfUserException() const
    {
        throw Test::TestIntfUserException();
    }
};

class TestActivationI : public Test::TestActivation
{
public:

    void activateServantLocator(bool activate, const Ice::Current& current)
    {
        if(activate)
        {
            current.adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorI, ""), "");
            current.adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorI, "category"), "category");
        }
        else
        {
            ServantLocatorPtr locator = current.adapter->removeServantLocator("");
            locator->deactivate("");
            locator = current.adapter->removeServantLocator("category");
            locator->deactivate("category");
        }
    }
};

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Collocated::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("Ice.Warn.Dispatch", "0");
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorI, ""), "");
    adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorI, "category"), "category");
    adapter->add(ICE_MAKE_SHARED(TestI), Ice::stringToIdentity("asm"));
    adapter->add(ICE_MAKE_SHARED(TestActivationI), Ice::stringToIdentity("test/activation"));

    Test::TestIntfPrxPtr allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Collocated)
