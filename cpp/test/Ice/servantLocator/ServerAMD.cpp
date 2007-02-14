// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <ServantLocatorI.h>
#include <TestCommon.h>
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

    virtual void
    throwTestIntfUserException() const
    {
        throw Test::TestIntfUserException();
    }
};

class TestServer : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    TestServer app;
    return app.main(argc, argv);
}

int
TestServer::run(int argc, char* argv[])
{
    communicator()->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000:udp");
    communicator()->getProperties()->setProperty("Ice.Warn.Dispatch", "0");

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
    
    adapter->addServantLocator(new ServantLocatorAMDI(""), "");
    adapter->addServantLocator(new ServantLocatorAMDI("category"), "category");
    adapter->add(new TestAMDI, communicator()->stringToIdentity("asm"));
    adapter->activate();
    adapter->waitForDeactivate();
    return EXIT_SUCCESS;
}
