//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceUtil/Options.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <TestHelper.h>
#include <Single.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class Publisher : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Publisher::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    IceUtilInternal::Options opts;
    opts.addOpt("", "cycle");

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        ostringstream os;
        os << argv[0] << ": " << e.reason;
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

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator->stringToProxy(managerProxy));
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    TopicPrx topic;
    while(true)
    {
        try
        {
            topic = manager->retrieve("single");
            break;
        }
        // This can happen if the replica group loses the majority
        // during retrieve. In this case we retry.
        catch(const Ice::UnknownException&)
        {
            continue;
        }
        catch(const IceStorm::NoSuchTopic& e)
        {
            ostringstream os;
            os << argv[0] << ": NoSuchTopic: " << e.name;
            throw invalid_argument(os.str());
        }
    }
    assert(topic);

    //
    // Get a publisher object, create a twoway proxy and then cast to
    // a Single object.
    //
    if(opts.isSet("cycle"))
    {
        Ice::ObjectPrx prx = topic->getPublisher()->ice_twoway();
        vector<SinglePrx> single;
        Ice::EndpointSeq endpoints = prx->ice_getEndpoints();
        for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
        {
            if((*p)->toString().substr(0, 3) != "udp")
            {
                Ice::EndpointSeq e;
                e.push_back(*p);
                single.push_back(SinglePrx::uncheckedCast(prx->ice_endpoints(e)));
            }
        }
        if(single.size() <= 1)
        {
            ostringstream os;
            os << argv[0] << ": Not enough endpoints in publisher proxy";
            throw invalid_argument(os.str());
        }
        int which = 0;
        for(int i = 0; i < 1000; ++i)
        {
            single[which]->event(i);
            which = (which + 1) % static_cast<int>(single.size());
        }
    }
    else
    {
        SinglePrx single = SinglePrx::uncheckedCast(topic->getPublisher()->ice_twoway());
        for(int i = 0; i < 1000; ++i)
        {
            single->event(i);
        }
    }
}

DEFINE_TEST(Publisher)
