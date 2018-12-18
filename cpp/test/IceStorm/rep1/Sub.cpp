// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Single.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class SingleI : public Single
{
public:

    SingleI() {}
    virtual void event(int, const Current&) {}
};

class Sub : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Sub::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    IceUtilInternal::Options opts;
    opts.addOpt("", "id", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "unsub");

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        ostringstream os;
        os << argv[0] << ": error: " << e.reason;
        throw invalid_argument(os.str());
    }

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

    Ice::ObjectPrx prx = adapter->add(new SingleI(), stringToIdentity(opts.optArg("id")));
    if(opts.isSet("unsub"))
    {
        topic->unsubscribe(prx);
    }
    else
    {
        IceStorm::QoS qos;
        qos["persistent"] = "true";
        topic->subscribeAndGetPublisher(qos, prx);
    }
}

DEFINE_TEST(Sub)
