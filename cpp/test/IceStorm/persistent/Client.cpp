// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "TestHelper.h"

#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class Client final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder ich = initialize(argc, argv, make_shared<Ice::Properties>("IceStormAdmin"));
    const auto& communicator = ich.communicator();
    auto properties = communicator->getProperties();
    auto managerProxy = properties->getIceProperty("IceStormAdmin.TopicManager.Default");
    if (managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    IceStorm::TopicManagerPrx manager(communicator, managerProxy);

    if (argc != 2)
    {
        throw invalid_argument("client create|check");
    }

    string action(argv[1]);
    if (action == "create")
    {
        //
        // Create topics
        //
        cerr << "creating topics and links..." << flush;
        optional<TopicPrx> linkTo;
        for (int i = 0; i < 10; ++i)
        {
            ostringstream topicName;
            topicName << "topic" << i;
            auto topic = manager->create(topicName.str());
            if (linkTo)
            {
                topic->link(linkTo, i + 1);
            }
            linkTo = topic;
        }
        cerr << "ok" << endl;

        //
        // Create a subscriber for each topic
        //
        cerr << "create a subscriber for each topic... ";
        for (int i = 0; i < 10; ++i)
        {
            ostringstream topicName;
            topicName << "topic" << i;
            auto topic = manager->retrieve(topicName.str());
            test(topic);

            ostringstream subscriber;
            subscriber << "subscriber" << i << ":default -h 10000";
            topic->subscribeAndGetPublisher(IceStorm::QoS(), ObjectPrx(communicator, subscriber.str()));
        }
        cerr << "ok" << endl;
    }
    else
    {
        test(action == "check");
    }
}

DEFINE_TEST(Client)
