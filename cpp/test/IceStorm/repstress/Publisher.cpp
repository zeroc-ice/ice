// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
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

class PublishThread : public IceUtil::Thread, public IceUtil::Mutex
{
public:

    PublishThread(const SinglePrx& single) :
        _single(single),
        _published(0),
        _destroy(false)
    {
    }

    virtual void run()
    {
        while(true)
        {
            {
                Lock sync(*this);
                if(_destroy)
                {
                    cout << _published << endl;
                    break;
                }
            }
            try
            {
                _single->event(_published);
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
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
        Lock sync(*this);
        _destroy = true;
    }

private:

    const SinglePrx _single;
    int _published;
    bool _destroy;
};
typedef IceUtil::Handle<PublishThread> PublishThreadPtr;

class Publisher : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Publisher::run(int argc, char** argv)
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

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator->stringToProxy(managerProxy));
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    TopicPrx topic = manager->retrieve("single");
    assert(topic);

    //
    // Get a publisher object, create a twoway proxy, disable
    // connection caching and then cast to a Single object.
    //
    SinglePrx single = SinglePrx::uncheckedCast(topic->getPublisher()->ice_twoway()->ice_connectionCached(false));

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("ControllerAdapter", "default");
    Ice::ObjectPrx controller = adapter->addWithUUID(new ControllerI);
    adapter->activate();
    cout << communicator->proxyToString(controller) << endl;

    PublishThreadPtr t = new PublishThread(single);
    t->start();

    communicator->waitForShutdown();

    t->destroy();
    t->getThreadControl().join();
}

DEFINE_TEST(Publisher)
