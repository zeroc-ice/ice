// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class EventI : public Event
{
public:

    virtual void
    pub(const string&, const Ice::Current& current)
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(_countMutex);

        if(++_count == 30 + 40 + 30)
        {
            current.adapter->getCommunicator()->shutdown();
        }
    }

    static IceUtil::Mutex* _countMutex;

private:

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
        "-b                   Use batch reliability.\n";
}

class Subscriber : public Test::TestHelper
{
public:

    void run(int, char**);
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

    PropertiesPtr properties = communicator->getProperties();
    string managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
    if(managerProxy.empty())
    {
        throw runtime_error("property `IceStormAdmin.TopicManager.Default' is not set");
    }

    ObjectPrx base = communicator->stringToProxy(managerProxy);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw runtime_error(os.str());
    }

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");
    //
    // Activate the servants.
    //
    ObjectPrx objFed1 = adapter->addWithUUID(new EventI());
    ObjectPrx objFed2 = adapter->addWithUUID(new EventI());
    ObjectPrx objFed3 = adapter->addWithUUID(new EventI());

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

    TopicPrx fed1 = manager->retrieve("fed1");
    TopicPrx fed2 = manager->retrieve("fed2");
    TopicPrx fed3 = manager->retrieve("fed3");

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
