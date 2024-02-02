//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <ServantLocatorI.h>
#include <TestHelper.h>
#include <Test.h>
#include <TestI.h>

using namespace std;
using namespace Ice;

class ServantLocatorI final : public Test::ServantLocatorI
{
public:

    ServantLocatorI(const string& category) : Test::ServantLocatorI(category)
    {
    }

protected:

    shared_ptr<Ice::Object> newServantAndCookie(shared_ptr<void>& cookie) const final
    {
        cookie = make_shared<Cookie>();
        return make_shared<TestI>();
    }

    void checkCookie(const shared_ptr<void>& cookie) const final
    {
        shared_ptr<Cookie> co = static_pointer_cast<Cookie>(cookie);
        test(co);
        test(co->message() == "blahblah");
    }

    void throwTestIntfUserException() const final
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
            current.adapter->addServantLocator(make_shared<ServantLocatorI>(""), "");
            current.adapter->addServantLocator(make_shared<ServantLocatorI>("category"), "category");
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
    adapter->addServantLocator(make_shared<ServantLocatorI>(""), "");
    adapter->addServantLocator(make_shared<ServantLocatorI>("category"), "category");
    adapter->add(make_shared<TestI>(), Ice::stringToIdentity("asm"));
    adapter->add(make_shared<TestActivationI>(), Ice::stringToIdentity("test/activation"));

    Test::TestIntfPrxPtr allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Collocated)
