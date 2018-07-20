// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Glacier2/Glacier2.h>
#include <IceStorm/IceStorm.h>
#include <IceGrid/IceGrid.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

namespace
{

class ClockI : public Clock
{
public:

#ifdef ICE_CPP11_MAPPING
    virtual void
    tick(string time, const Ice::Current&)
    {
        cout << time << endl;
    }
#else
    virtual void
    tick(const string& time, const Ice::Current&)
    {
        cout << time << endl;
    }
#endif
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
        _factory = ICE_MAKE_SHARED(Glacier2::SessionFactoryHelper, ICE_MAKE_SHARED(SessionCallbackI));
        return EXIT_SUCCESS;
    }

private:

    Glacier2::SessionHelperPtr _session;
    Glacier2::SessionFactoryHelperPtr _factory;
    Ice::InitializationData _initData;
};

} // Anonymous namespace end

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
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
        IceStorm::TopicManagerPrxPtr manager =
                    ICE_UNCHECKED_CAST(IceStorm::TopicManagerPrx, communicator->stringToProxy("test:default -p 12010"));

        IceStorm::QoS qos;
        IceStorm::TopicPrxPtr topic;
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
        Ice::ObjectPrxPtr subscriber = adapter->addWithUUID(ICE_MAKE_SHARED(ClockI));
        adapter->activate();
#ifdef ICE_CPP11_MAPPING
        assert(!topic);
#else
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
#endif
        cout << "ok" << endl;
    }

    {
        cout << "Testing IceGrid stub... " << flush;

        Ice::ObjectPrxPtr base = communicator->stringToProxy("test:" + helper->getTestEndpoint());
        IceGrid::RegistryPrxPtr registry = ICE_UNCHECKED_CAST(IceGrid::RegistryPrx, base);
        IceGrid::AdminSessionPrxPtr session;
        IceGrid::AdminPrxPtr admin;
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
#ifdef ICE_CPP11_MAPPING
        assert(!admin);
#else
        try
        {
            admin = session->getAdmin();
            test(false);
        }
        catch(const IceUtil::NullHandleException&)
        {
        }
#endif
        cout << "ok" << endl;
    }
}
