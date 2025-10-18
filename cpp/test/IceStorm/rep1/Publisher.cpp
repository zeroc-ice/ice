// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Options.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "Single.h"
#include "TestHelper.h"

#include <stdexcept>

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
    Ice::CommunicatorHolder ich = initialize(argc, argv, make_shared<Ice::Properties>("IceStormAdmin"));
    const auto& communicator = ich.communicator();
    IceInternal::Options opts;
    opts.addOpt("", "cycle");

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch (const IceInternal::BadOptException& e)
    {
        ostringstream os;
        os << argv[0] << ": " << e.what();
        throw invalid_argument(os.str());
    }

    auto managerProxy = communicator->getProperties()->getIceProperty("IceStormAdmin.TopicManager.Default");
    if (managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    IceStorm::TopicManagerPrx manager(communicator, managerProxy);
    optional<TopicPrx> topic;
    while (true)
    {
        try
        {
            topic = manager->retrieve("single");
            break;
        }
        // This can happen if the replica group loses the majority
        // during retrieve. In this case we retry.
        catch (const Ice::UnknownException&)
        {
            continue;
        }
        catch (const IceStorm::NoSuchTopic& e)
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
    if (opts.isSet("cycle"))
    {
        auto prx = uncheckedCast<SinglePrx>(topic->getPublisher()->ice_twoway());
        vector<optional<SinglePrx>> single;
        auto endpoints = prx->ice_getEndpoints();
        for (const auto& p : endpoints)
        {
            if (p->toString().substr(0, 3) != "udp")
            {
                Ice::EndpointSeq e;
                e.push_back(p);
                single.emplace_back(prx->ice_endpoints(e));
            }
        }
        if (single.size() <= 1)
        {
            ostringstream os;
            os << argv[0] << ": Not enough endpoints in publisher proxy";
            throw invalid_argument(os.str());
        }
        size_t which = 0;
        for (size_t i = 0; i < 1000; ++i)
        {
            single[which]->event(static_cast<int>(i));
            which = (which + 1) % single.size();
        }
    }
    else
    {
        auto single = uncheckedCast<SinglePrx>(topic->getPublisher()->ice_twoway());
        for (int i = 0; i < 1000; ++i)
        {
            single->event(i);
        }
    }
}

DEFINE_TEST(Publisher)
