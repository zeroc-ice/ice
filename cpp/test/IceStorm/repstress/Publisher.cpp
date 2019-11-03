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

class PublishThread final
{
public:

    explicit PublishThread(shared_ptr<SinglePrx> single) :
        _single(std::move(single)),
        _published(0),
        _destroy(false)
    {
    }

    void run()
    {
        while(true)
        {
            {
                lock_guard<mutex> log(_mutex);
                if(_destroy)
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
            catch(const Ice::UnknownException&)
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

    const shared_ptr<SinglePrx> _single;
    int _published;
    bool _destroy;
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
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    auto properties = communicator->getProperties();
    string managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
    if(managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    auto manager = checkedCast<IceStorm::TopicManagerPrx>(
        communicator->stringToProxy(managerProxy));
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    auto topic = manager->retrieve("single");
    assert(topic);

    //
    // Get a publisher object, create a twoway proxy, disable
    // connection caching and then cast to a Single object.
    //
    auto single = uncheckedCast<SinglePrx>(topic->getPublisher()->ice_twoway()->ice_connectionCached(false));

    auto adapter = communicator->createObjectAdapterWithEndpoints("ControllerAdapter", "default");
    auto controller = adapter->addWithUUID(make_shared<ControllerI>());
    adapter->activate();
    cout << communicator->proxyToString(controller) << endl;

    PublishThread pt(move(single));
    auto fut = std::async(launch::async, [&pt]{ pt.run(); });

    communicator->waitForShutdown();

    pt.destroy();
    fut.get();
}

DEFINE_TEST(Publisher)
