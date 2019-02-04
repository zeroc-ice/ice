//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Single.h>
#include <Controller.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class ControllerI : public Controller
{
public:

    virtual void stop(const Ice::Current& c)
    {
        c.adapter->getCommunicator()->shutdown();
    }
};

class SingleI : public Single, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SingleI() :
        _nevents(0)
    {
    }

    virtual void
    event(int, const Current&)
    {
        Lock sync(*this);
        ++_nevents;
    }

    int
    nevents() const
    {
        Lock sync(*this);
        return _nevents;
    }

private:

    int _nevents;
};
typedef IceUtil::Handle<SingleI> SingleIPtr;

class Subscriber : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Subscriber::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    PropertiesPtr properties = communicator->getProperties();
    string managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
    if(managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    ObjectPrx base = communicator->stringToProxy(managerProxy);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default");
    TopicPrx topic = manager->retrieve("single");

    SingleIPtr sub = new SingleI();
    Ice::ObjectPrx prx = adapter->addWithUUID(sub);
    Ice::ObjectPrx control = adapter->addWithUUID(new ControllerI);

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
