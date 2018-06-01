// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ServantLocatorI.h>
#include <TestHelper.h>
#include <TestAMDI.h>

using namespace std;
using namespace Ice;

class ServantLocatorAMDI : public Test::ServantLocatorI
{
public:

    ServantLocatorAMDI(const string& category) : Test::ServantLocatorI(category)
    {
    }

protected:

#ifdef ICE_CPP11_MAPPING
    virtual Ice::ObjectPtr
    newServantAndCookie(shared_ptr<void>& cookie) const
    {
        cookie = make_shared<CookieI>();
        return make_shared<TestAMDI>();
    }

    virtual void
    checkCookie(const shared_ptr<void>& cookie) const
    {
        auto co = static_pointer_cast<Test::Cookie>(cookie);
        test(co);
        test(co->message() == "blahblah");
    }
#else
    virtual Ice::ObjectPtr
    newServantAndCookie(Ice::LocalObjectPtr& cookie) const
    {
        cookie = new CookieI();
        return new TestAMDI();
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
            current.adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorAMDI, ""), "");
            current.adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorAMDI, "category"), "category");
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

class ServerAMD : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
ServerAMD::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    communicator->getProperties()->setProperty("Ice.Warn.Dispatch", "0");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");

    adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorAMDI, ""), "");
    adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorAMDI, "category"), "category");
    adapter->add(ICE_MAKE_SHARED(TestAMDI), Ice::stringToIdentity("asm"));
    adapter->add(ICE_MAKE_SHARED(TestActivationI), Ice::stringToIdentity("test/activation"));
    adapter->activate();
    serverReady();
    adapter->waitForDeactivate();
}

DEFINE_TEST(ServerAMD)
