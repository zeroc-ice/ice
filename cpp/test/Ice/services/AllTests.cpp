// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Glacier2/Glacier2.h>
#include <IceStorm/IceStorm.h>
#include <IceGrid/IceGrid.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

namespace
{

class ClockI : public Clock
{
public:

    virtual void
    tick(const string& time, const Ice::Current&)
    {
        cout << time << endl;
    }
};

class SessionCallbackI : public Glacier2::SessionCallback
{

public:

    virtual void
    connected(const Glacier2::SessionHelperPtr&)
    {
    }

    virtual void
    disconnected(const Glacier2::SessionHelperPtr&)
    {
    }

    virtual void
    connectFailed(const Glacier2::SessionHelperPtr&, const Ice::Exception& ex)
    {
    }

    virtual void
    createdCommunicator(const Glacier2::SessionHelperPtr& session)
    {
    }
};

class SessionHelperClient
{
public:

    int run(int argc, char* argv[])
    {
        _factory = new Glacier2::SessionFactoryHelper(new SessionCallbackI());
        return EXIT_SUCCESS;
    }

private:

    Glacier2::SessionHelperPtr _session;
    Glacier2::SessionFactoryHelperPtr _factory;
    Ice::InitializationData _initData;
};

} // Anonymous namespace end

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    {
        cout << "Testing Glacier2 stub... " << flush;
        char** argv = 0;
        int argc = 0;
        SessionHelperClient client;
        client.run(argc, argv);
        cout << "ok" << endl;
    }

    {
        cout << "Testing IceStorm stub... " << flush;
        IceStorm::TopicManagerPrx manager =
                    IceStorm::TopicManagerPrx::uncheckedCast(communicator->stringToProxy("test:default -p 12010"));

        IceStorm::QoS qos;
        IceStorm::TopicPrx topic;
        string topicName = "time";

        try
        {
            topic = manager->retrieve(topicName);
            test(false);
        }
        catch(const IceStorm::NoSuchTopic&)
        {
            test(false);
        }
        catch(const Ice::LocalException&)
        {
        }

        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("subscriber" ,"tcp");
        Ice::ObjectPrx subscriber = adapter->addWithUUID(new ClockI);
        adapter->activate();
        try
        {
            topic->subscribeAndGetPublisher(qos, subscriber);
            test(false);
        }
        catch(const IceStorm::AlreadySubscribed&)
        {
            test(false);
        }
        catch(const IceUtil::NullHandleException&)
        {
        }
        cout << "ok" << endl;
    }

    {
        cout << "Testing IceGrid stub... " << flush;

        Ice::ObjectPrx base = communicator->stringToProxy("test:default -p 12010");
        IceGrid::RegistryPrx registry = IceGrid::RegistryPrx::uncheckedCast(base);
        IceGrid::AdminSessionPrx session;
        IceGrid::AdminPrx admin;
        try
        {
            session = registry->createAdminSession("username", "password");
            test(false);
        }
        catch(const IceGrid::PermissionDeniedException&)
        {
            test(false);
        }
        catch(const Ice::LocalException&)
        {
        }

        try
        {
            admin = session->getAdmin();
            test(false);
        }
        catch(const IceUtil::NullHandleException&)
        {
        }
        cout << "ok" << endl;
    }
}
