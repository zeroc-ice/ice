//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        virtual void tick(string time, const Ice::Current&) { cout << time << endl; }
    };

    class SessionCallbackI : public Glacier2::SessionCallback
    {
    public:
        virtual void connected(const Glacier2::SessionHelperPtr&) {}

        virtual void disconnected(const Glacier2::SessionHelperPtr&) {}

        virtual void connectFailed(const Glacier2::SessionHelperPtr&, std::exception_ptr) {}

        virtual void createdCommunicator(const Glacier2::SessionHelperPtr&) {}
    };

    class SessionHelperClient
    {
    public:
        int run(int, char*[])
        {
            _factory = make_shared<Glacier2::SessionFactoryHelper>(make_shared<SessionCallbackI>());
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
        IceStorm::TopicManagerPrx manager(communicator, "test:default -p 12010");

        IceStorm::QoS qos;
        optional<IceStorm::TopicPrx> topic;
        string topicName = "time";

        try
        {
            topic = manager->retrieve(topicName);
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
        assert(!topic);
        cout << "ok" << endl;
    }

    {
        cout << "Testing IceGrid stub... " << flush;

        IceGrid::RegistryPrx registry(communicator, "test:" + helper->getTestEndpoint());
        optional<IceGrid::AdminSessionPrx> session;
        optional<IceGrid::AdminPrx> admin;
        try
        {
            session = registry->createAdminSession("username", "password");
            test(false);
        }
        catch (const IceGrid::PermissionDeniedException&)
        {
            test(false);
        }
        catch (const Ice::LocalException&)
        {
        }
        assert(!admin);
        cout << "ok" << endl;
    }
}
