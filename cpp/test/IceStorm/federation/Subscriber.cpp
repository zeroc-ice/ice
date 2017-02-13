// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Event.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class EventI : public Event
{
public:

    EventI(const CommunicatorPtr& communicator) :
        _communicator(communicator)
    {
    }

    virtual void
    pub(const string&, const Ice::Current&)
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(_countMutex);

        if(++_count == 30 + 40 + 30)
        {
            _communicator->shutdown();
        }
    }

    static IceUtil::Mutex* _countMutex;

private:

    CommunicatorPtr _communicator;

    static int _count;
};

typedef IceUtil::Handle<EventI> EventIPtr;

int EventI::_count = 0;
IceUtil::Mutex* EventI::_countMutex = 0;

namespace
{

class Init
{
public:

    Init()
    {
        EventI::_countMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete EventI::_countMutex;
        EventI::_countMutex = 0;
    }
};

Init init;

}

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

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
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
            return EXIT_SUCCESS;
        }
        else if(argv[idx][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[idx] << "'" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

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

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");
    EventIPtr eventFed1 = new EventI(communicator);
    EventIPtr eventFed2 = new EventI(communicator);
    EventIPtr eventFed3 = new EventI(communicator);

    //
    // Activate the servants.
    //
    ObjectPrx objFed1 = adapter->addWithUUID(eventFed1);
    ObjectPrx objFed2 = adapter->addWithUUID(eventFed2);
    ObjectPrx objFed3 = adapter->addWithUUID(eventFed3);

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

    TopicPrx fed1;
    TopicPrx fed2;
    TopicPrx fed3;

    try
    {
        fed1 = manager->retrieve("fed1");
        fed2 = manager->retrieve("fed2");
        fed3 = manager->retrieve("fed3");
    }
    catch(const IceStorm::NoSuchTopic& e)
    {
        cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
        return EXIT_FAILURE;
    }

    fed1->subscribeAndGetPublisher(qos, objFed1);
    fed2->subscribeAndGetPublisher(qos, objFed2);
    fed3->subscribeAndGetPublisher(qos, objFed3);

    adapter->activate();
    communicator->waitForShutdown();

    fed1->unsubscribe(objFed1);
    fed2->unsubscribe(objFed2);
    fed3->unsubscribe(objFed3);

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
