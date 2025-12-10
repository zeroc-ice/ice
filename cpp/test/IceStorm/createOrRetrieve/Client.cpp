// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "Test.h"
#include "TestHelper.h"

#include <chrono>
#include <future>
#include <stdexcept>
#include <thread>

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
    Ice::CommunicatorPtr communicator = initialize(argc, argv, make_shared<Ice::Properties>("IceStormAdmin"));
    Ice::CommunicatorHolder communicatorHolder{communicator};
    auto properties = communicator->getProperties();
    auto managerProxy = properties->getIceProperty("IceStormAdmin.TopicManager.Default");
    if (managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    IceStorm::TopicManagerPrx manager(communicator, managerProxy);

    cout << "testing createOrRetrieve operation... " << flush;

    //
    // Test 1: Creating a topic via createOrRetrieve when it doesn't exist
    //
    {
        const string topicName = "createOrRetrieve-topic1";

        // Ensure topic doesn't exist
        try
        {
            manager->retrieve(topicName);
            test(false); // Topic should not exist
        }
        catch (const NoSuchTopic&)
        {
            // Expected - topic doesn't exist
        }

        // Create topic using createOrRetrieve
        auto topic1 = manager->createOrRetrieve(topicName);
        test(topic1);
        test(topic1->getName() == topicName);

        // Verify topic was created
        auto verifyTopic = manager->retrieve(topicName);
        test(verifyTopic);
        test(verifyTopic->getName() == topicName);

        // Clean up
        topic1->destroy();
    }

    //
    // Test 2: Retrieving an existing topic via createOrRetrieve
    //
    {
        const string topicName = "createOrRetrieve-topic2";

        // Create topic first using regular create
        auto originalTopic = manager->create(topicName);
        test(originalTopic);

        // Now use createOrRetrieve - should retrieve the existing topic
        auto retrievedTopic = manager->createOrRetrieve(topicName);
        test(retrievedTopic);
        test(retrievedTopic->getName() == topicName);

        // Verify both proxies refer to the same topic
        test(originalTopic->getName() == retrievedTopic->getName());

        // Clean up
        originalTopic->destroy();
    }

    //
    // Test 3: Concurrent calls to createOrRetrieve for the same topic name
    //
    {
        const string topicName = "createOrRetrieve-topic3";

        // Ensure topic doesn't exist
        try
        {
            manager->retrieve(topicName);
            test(false); // Topic should not exist
        }
        catch (const NoSuchTopic&)
        {
            // Expected - topic doesn't exist
        }

        // Launch multiple concurrent createOrRetrieve calls
        const int numThreads = 10;
        vector<future<optional<TopicPrx>>> futures;
        futures.reserve(numThreads);

        for (int i = 0; i < numThreads; ++i)
        {
            futures.push_back(async(
                launch::async,
                [&manager, &topicName]()
                {
                    try
                    {
                        return manager->createOrRetrieve(topicName);
                    }
                    catch (const exception& ex)
                    {
                        cerr << "Exception in concurrent createOrRetrieve: " << ex.what() << endl;
                        throw;
                    }
                }));
        }

        // Collect all results
        for (auto& f : futures)
        {
            auto topic = f.get();
            test(topic);
            test(topic->getName() == topicName);
        }

        // Verify only one topic was actually created
        auto allTopics = manager->retrieveAll();
        test(allTopics.size() == 1);
        test(allTopics.find(topicName) != allTopics.end());
        test(allTopics[topicName]->getName() == topicName);

        // Clean up
        allTopics[topicName]->destroy();
    }

    cout << "ok" << endl;
}

DEFINE_TEST(Client)
