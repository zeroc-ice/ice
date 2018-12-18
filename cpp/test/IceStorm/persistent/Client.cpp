// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
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

    IceStorm::TopicManagerPrx manager =
        IceStorm::TopicManagerPrx::checkedCast(communicator->stringToProxy(managerProxy));
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    if(argc != 2)
    {
        throw invalid_argument("client create|check");
    }

    string action(argv[1]);
    if(action == "create")
    {
        //
        // Create topics
        //
        cerr << "creating topics and links..." << flush;
        TopicPrx linkTo = 0;
        for(int i = 0; i < 10; ++i)
        {
            ostringstream topicName;
            topicName << "topic" << i;
            TopicPrx topic = manager->create(topicName.str());
            if(linkTo)
            {
                topic->link(linkTo, i + 1);
            }
            linkTo = topic;
        }
        cerr << "ok" << endl;

        //
        // Create a subscriber for each topic
        //
        cerr << "create a susbscriber for each topic... ";
        for(int i = 0; i < 10; ++i)
        {
            ostringstream topicName;
            topicName << "topic" << i;
            TopicPrx topic = manager->retrieve(topicName.str());
            test(topic);

            ostringstream subscriber;
            subscriber << "subscriber" << i << ":default -h 10000";
            topic->subscribeAndGetPublisher(IceStorm::QoS(), communicator->stringToProxy(subscriber.str()));
        }
        cerr << "ok" << endl;
    }
    else
    {
        test(action == "check");
    }
}

DEFINE_TEST(Client)
