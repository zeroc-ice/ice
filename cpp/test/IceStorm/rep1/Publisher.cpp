//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceUtil/Options.h>
#include <IceStorm/IceStorm.h>
#include <TestHelper.h>
#include <Single.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class Publisher final : public Test::TestHelper
{
public:

    void run(int, char**) override;
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

    auto properties = communicator->getProperties();
    auto managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
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

    shared_ptr<TopicPrx> topic;
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
        auto prx = uncheckedCast<SinglePrx>(topic->getPublisher()->ice_twoway());
        vector<shared_ptr<SinglePrx>> single;
        auto endpoints = prx->ice_getEndpoints();
        for(const auto& p: endpoints)
        {
            if(p->toString().substr(0, 3) != "udp")
            {
                Ice::EndpointSeq e;
                e.push_back(p);
                single.push_back(prx->ice_endpoints(e));
            }
        }
        if(single.size() <= 1)
        {
            ostringstream os;
            os << argv[0] << ": Not enough endpoints in publisher proxy";
            throw invalid_argument(os.str());
        }
        size_t which = 0;
        for(size_t i = 0; i < 1000; ++i)
        {
            single[which]->event(static_cast<int>(i));
            which = (which + 1) % single.size();
        }
    }
    else
    {
        auto single = uncheckedCast<SinglePrx>(topic->getPublisher()->ice_twoway());
        for(int i = 0; i < 1000; ++i)
        {
            single->event(i);
        }
    }
}

DEFINE_TEST(Publisher)
