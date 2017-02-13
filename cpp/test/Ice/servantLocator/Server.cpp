// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <ServantLocatorI.h>
#include <TestCommon.h>
#include <TestI.h>

DEFINE_TEST("server")

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
    virtual shared_ptr<::Ice::Object>
    newServantAndCookie(shared_ptr<void>& cookie) const
    {
        cookie = make_shared<CookieI>();
        return make_shared<TestI>();
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

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0) + ":udp");
    communicator->getProperties()->setProperty("Ice.Warn.Dispatch", "0");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");

    adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorI, ""), "");
    adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorI, "category"), "category");
    adapter->add(ICE_MAKE_SHARED(TestI), Ice::stringToIdentity("asm"));
    adapter->add(ICE_MAKE_SHARED(TestActivationI), Ice::stringToIdentity("test/activation"));
    adapter->activate();
    TEST_READY
    adapter->waitForDeactivate();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif
    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }
}
