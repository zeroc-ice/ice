// Copyright (c) ZeroC, Inc.

#include "Glacier2/Glacier2.h"
#include "Ice/Ice.h"
#include "IceGrid/IceGrid.h"
#include "IceStorm/IceStorm.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

namespace
{
    class ClockI : public Clock
    {
    public:
        void tick(string time, const Ice::Current&) override { cout << time << endl; }
    };
}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    {
        cout << "Testing Glacier2 stub... " << flush;
        Glacier2::RouterPrx router(communicator, "Glacier2/router: " + helper->getTestEndpoint());

        try
        {
            router->createSession("username", "password");
            test(false);
        }
        catch (const Glacier2::PermissionDeniedException&)
        {
            test(false);
        }
        catch (const Ice::LocalException&)
        {
        }

        cout << "ok" << endl;
    }

    {
        cout << "Testing IceStorm stub... " << flush;
        IceStorm::TopicManagerPrx manager(communicator, "test:default -p 12010");

        IceStorm::QoS qos;
        string topicName = "time";

        try
        {
            manager->retrieve(topicName);
            test(false);
        }
        catch (const IceStorm::NoSuchTopic&)
        {
            test(false);
        }
        catch (const Ice::LocalException&)
        {
        }

        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("subscriber", "tcp");
        Ice::ObjectPrx subscriber(adapter->addWithUUID(std::make_shared<ClockI>()));
        adapter->activate();
        cout << "ok" << endl;
    }

    {
        cout << "Testing IceGrid stub... " << flush;

        IceGrid::RegistryPrx registry(communicator, "test:" + helper->getTestEndpoint());
        optional<IceGrid::AdminPrx> admin;
        try
        {
            registry->createAdminSession("username", "password");
            test(false);
        }
        catch (const IceGrid::PermissionDeniedException&)
        {
            test(false);
        }
        catch (const Ice::LocalException&)
        {
        }
        cout << "ok" << endl;
    }
}
