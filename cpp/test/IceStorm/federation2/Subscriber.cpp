//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Event.h>
#include <TestHelper.h>

#include <fcntl.h>

#ifdef _WIN32
#   include <io.h>
#else
#   include <sys/types.h>
#   include <sys/stat.h>
#endif

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
        lock_guard<mutex> lg(countMutex);

        if(++_count == 10)
        {
            current.adapter->getCommunicator()->shutdown();
        }
    }

    static mutex countMutex;

private:

    static int _count;
};

int EventI::_count = 0;
mutex EventI::countMutex;

void
usage(const char* appName)
{
    cerr << "Usage: " << appName << " [options]\n";
    cerr <<
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-b                   Use batch reliability.\n"
        ;
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
            os << argv[0] << ": unknown option `" << argv[idx] << "'";
            throw invalid_argument(os.str());
        }
    }

    auto properties = communicator->getProperties();
    auto managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
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

    auto adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");

    //
    // Activate the servants.
    //
    auto obj = adapter->addWithUUID(make_shared<EventI>());

    IceStorm::QoS qos;
    if(batch)
    {
        obj = obj->ice_batchOneway();
    }
    else
    {
        obj = obj->ice_oneway();
    }

    auto fed1 = manager->retrieve("fed1");

    fed1->subscribeAndGetPublisher(qos, obj);

    adapter->activate();

    communicator->waitForShutdown();

    fed1->unsubscribe(obj);
}

DEFINE_TEST(Subscriber)
