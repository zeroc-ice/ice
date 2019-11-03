//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Single.h>
#include <Controller.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class ControllerI final : public Controller
{
public:

    void stop(const Ice::Current& c) override
    {
        c.adapter->getCommunicator()->shutdown();
    }
};

class SingleI final : public Single
{
public:

    void
    event(int, const Current&) override
    {
        ++_nevents;
    }

    int
    nevents() const
    {
        return _nevents;
    }

private:

    atomic_int _nevents = 0;
};

class Subscriber final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
Subscriber::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    auto properties = communicator->getProperties();
    string managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
    if(managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    auto base = communicator->stringToProxy(managerProxy);
    auto manager = checkedCast<IceStorm::TopicManagerPrx>(base);
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    auto adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default");
    auto topic = manager->retrieve("single");

    auto sub = make_shared<SingleI>();
    auto prx = adapter->addWithUUID(sub);
    auto control = adapter->addWithUUID(make_shared<ControllerI>());

    IceStorm::QoS qos;

    while(true)
    {
        try
        {
            topic->subscribeAndGetPublisher(qos, prx);
            break;
        }
        // If we're already subscribed then we're done (previously we
        // got an UnknownException which succeeded).
        catch(const IceStorm::AlreadySubscribed&)
        {
            break;
        }
        // This can happen if the replica group loses the majority
        // during subscription. In this case we retry.
        catch(const Ice::UnknownException&)
        {
        }
    }

    adapter->activate();
    cout << communicator->proxyToString(control) << endl;

    communicator->waitForShutdown();

    cout << sub->nevents() << endl;
}

DEFINE_TEST(Subscriber)
