// Copyright (c) ZeroC, Inc.

#include "Controller.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "Single.h"
#include "TestHelper.h"

#include <stdexcept>
#include <thread>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class ControllerI final : public Controller
{
public:
    void stop(const Ice::Current& c) override { c.adapter->getCommunicator()->shutdown(); }
};

class PublishThread final
{
public:
    explicit PublishThread(SinglePrx single) : _single(std::move(single)) {}

    void run()
    {
        while (true)
        {
            {
                lock_guard<mutex> log(_mutex);
                if (_destroy)
                {
                    cout << _published << endl;
                    break;
                }
            }
            try
            {
                _single->event(_published);
                this_thread::sleep_for(1ms);
            }
            catch (const Ice::UnknownException&)
            {
                // This is expected if we publish to a replica that is
                // going down.
                continue;
            }
            ++_published;
        }
    }

    void destroy()
    {
        lock_guard<mutex> log(_mutex);
        _destroy = true;
    }

private:
    const SinglePrx _single;
    int _published{0};
    bool _destroy{false};
    mutex _mutex;
};

class Publisher final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Publisher::run(int argc, char** argv)
{
    Ice::CommunicatorHolder ich = initialize(argc, argv, make_shared<Ice::Properties>(vector<string>{"IceStormAdmin"}));
    const auto& communicator = ich.communicator();
    auto properties = communicator->getProperties();
    string managerProxy = properties->getIceProperty("IceStormAdmin.TopicManager.Default");
    if (managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    IceStorm::TopicManagerPrx manager(communicator, managerProxy);

    auto topic = manager->retrieve("single");
    assert(topic);

    //
    // Get a publisher object, create a twoway proxy, disable
    // connection caching and then cast to a Single object.
    //
    auto single = uncheckedCast<SinglePrx>(topic->getPublisher()->ice_twoway()->ice_connectionCached(false));
    auto adapter = communicator->createObjectAdapterWithEndpoints("ControllerAdapter", "tcp");
    auto controller = adapter->addWithUUID(make_shared<ControllerI>());
    adapter->activate();
    cout << communicator->proxyToString(controller) << endl;

    PublishThread pt(std::move(single));
    auto fut = std::async(launch::async, [&pt] { pt.run(); });

    communicator->waitForShutdown();

    pt.destroy();
    fut.get();
}

DEFINE_TEST(Publisher)
