// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
#include <TestCommon.h>

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

int
run(int, char* argv[], const CommunicatorPtr& communicator)
{
    PropertiesPtr properties = communicator->getProperties();
    const char* managerProxyProperty = "IceStormAdmin.TopicManager.Default";
    string managerProxy = properties->getProperty(managerProxyProperty);
    if(managerProxy.empty())
    {
        cerr << argv[0] << ": property `" << managerProxyProperty << "' is not set" << endl;
        return EXIT_FAILURE;
    }

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator->stringToProxy(managerProxy));
    if(!manager)
    {
        cerr << argv[0] << ": `" << managerProxy << "' is not running" << endl;
        return EXIT_FAILURE;
    }

    TopicPrx topic;
    try
    {
        topic = manager->retrieve("single");
    }
    catch(const NoSuchTopic& e)
    {
        cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
        return EXIT_FAILURE;

    }
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

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;
    InitializationData initData = getTestInitData(argc, argv);
    try
    {
        communicator = initialize(argc, argv, initData);
        status = run(argc, argv, communicator);
    }
    catch(const Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        communicator->destroy();
    }

    return status;
}
