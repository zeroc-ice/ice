//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Event.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class EventI final : public Event
{
public:

    void
    pub(string, const Ice::Current& current) override
    {
        if(++_count == 30 + 40 + 30)
        {
            current.adapter->getCommunicator()->shutdown();
        }
    }

private:

    static atomic_int _count;
};

atomic_int EventI::_count = 0;

void
usage(const char* appName)
{
    cerr << "Usage: " << appName << " [options]\n";
    cerr <<
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-b                   Use batch reliability.\n";
}

class Subscriber final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
Subscriber::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    bool batch = false;

    int idx = 1;
    while(idx < argc)
    {
        if(strcmp(argv[idx], "-b") == 0)
        {
            batch = true;

            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
        {
            usage(argv[0]);
            return;
        }
        else if(argv[idx][0] == '-')
        {
            usage(argv[0]);
            ostringstream os;
            os << argv[0] <<": unknown option `" << argv[idx] << "'";
            throw invalid_argument(os.str());
        }
    }

    auto properties = communicator->getProperties();
    auto managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
    if(managerProxy.empty())
    {
        throw runtime_error("property `IceStormAdmin.TopicManager.Default' is not set");
    }

    auto base = communicator->stringToProxy(managerProxy);
    auto manager = checkedCast<IceStorm::TopicManagerPrx>(base);
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw runtime_error(os.str());
    }

    auto adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");
    //
    // Activate the servants.
    //
    auto objFed1 = adapter->addWithUUID(make_shared<EventI>());
    auto objFed2 = adapter->addWithUUID(make_shared<EventI>());
    auto objFed3 = adapter->addWithUUID(make_shared<EventI>());

    IceStorm::QoS qos;
    if(batch)
    {
        objFed1 = objFed1->ice_batchOneway();
        objFed2 = objFed1->ice_batchOneway();
        objFed3 = objFed1->ice_batchOneway();
    }
    else
    {
        objFed1 = objFed1->ice_oneway();
        objFed2 = objFed1->ice_oneway();
        objFed3 = objFed1->ice_oneway();
    }

    auto fed1 = manager->retrieve("fed1");
    auto fed2 = manager->retrieve("fed2");
    auto fed3 = manager->retrieve("fed3");

    fed1->subscribeAndGetPublisher(qos, objFed1);
    fed2->subscribeAndGetPublisher(qos, objFed2);
    fed3->subscribeAndGetPublisher(qos, objFed3);

    adapter->activate();
    communicator->waitForShutdown();

    fed1->unsubscribe(objFed1);
    fed2->unsubscribe(objFed2);
    fed3->unsubscribe(objFed3);
}

DEFINE_TEST(Subscriber)
