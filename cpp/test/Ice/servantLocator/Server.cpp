// Copyright (c) ZeroC, Inc.

#include "ServantLocatorI.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;
using namespace Ice;

class ServantLocatorI final : public Test::ServantLocatorI
{
public:
    ServantLocatorI(const string& category) : Test::ServantLocatorI(category) {}

protected:
    Ice::ObjectPtr newServantAndCookie(shared_ptr<void>& cookie) const final
    {
        cookie = make_shared<Cookie>();
        return make_shared<TestI>();
    }

    void checkCookie(const shared_ptr<void>& cookie) const final
    {
        auto co = static_pointer_cast<Cookie>(cookie);
        test(co);
        test(co->message() == "blahblah");
    }

    void throwTestIntfUserException() const final { throw Test::TestIntfUserException(); }
};

class TestActivationI : public Test::TestActivation
{
public:
    void activateServantLocator(bool activate, const Ice::Current& current) override
    {
        if (activate)
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

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    communicator->getProperties()->setProperty("Ice.Warn.Dispatch", "0");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");

    adapter->addServantLocator(make_shared<ServantLocatorI>(""), "");
    adapter->addServantLocator(make_shared<ServantLocatorI>("category"), "category");
    adapter->add(std::make_shared<TestI>(), Ice::stringToIdentity("asm"));
    adapter->add(std::make_shared<TestActivationI>(), Ice::stringToIdentity("test/activation"));
    adapter->activate();
    serverReady();
    adapter->waitForDeactivate();
}

DEFINE_TEST(Server)
