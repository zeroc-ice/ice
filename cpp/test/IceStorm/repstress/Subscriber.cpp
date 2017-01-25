// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
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

    ObjectPrx base = communicator->stringToProxy(managerProxy);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
        cerr << argv[0] << ": `" << managerProxy << "' is not running" << endl;
        return EXIT_FAILURE;
    }

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default");

    TopicPrx topic;
    try
    {
        topic = manager->retrieve("single");
    }
    catch(const IceStorm::NoSuchTopic& e)
    {
        cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
        return EXIT_FAILURE;
    }

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
