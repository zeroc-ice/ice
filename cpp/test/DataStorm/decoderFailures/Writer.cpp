// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"
#include "TestKey.h"

using namespace DataStorm;
using namespace std;

class Writer : public Test::TestHelper
{
public:
    Writer() : Test::TestHelper(false) {}

    void run(int, char**) override;
};

void ::Writer::run(int argc, char* argv[])
{
    Node node(argc, argv);

    WriterConfig config;
    config.sampleCount = -1;
    config.clearHistory = ClearHistoryPolicy::Never;

    cout << "testing an undecodable key in an element spec sent to a reader... " << flush;
    {
        // One element carries both keys, so the any-key reader is answered with a spec for a key it can decode and a
        // spec for a key it cannot.
        Topic<TestKey, string> topic(node, "announcedKeys");
        auto writer = makeMultiKeyWriter(topic, {TestKey{"good"}, TestKey{"poison"}}, "", config);

        writer.waitForReaders(1);
        writer.add(TestKey{"good"}, "announced");
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    cout << "testing an undecodable key in an element spec... " << flush;
    {
        // An any-key writer publishes the key with each sample rather than announcing it, so the reader's keys travel
        // to this writer inline in the element specs and are decoded here.
        Topic<TestKey, string> topic(node, "anyKey");
        auto writer = makeAnyKeyWriter(topic, "", config);

        writer.waitForReaders(1);
        writer.add(TestKey{"good"}, "spec");
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    cout << "testing an undecodable key filter criteria... " << flush;
    {
        Topic<TestKey, string> topic(node, "keyFilter");
        topic.setKeyFilter<TestKey>(
            "named",
            [](const TestKey& criteria) { return [criteria](const TestKey& key) { return key == criteria; }; });

        auto writer = makeSingleKeyWriter(topic, TestKey{"good"}, "", config);

        writer.waitForReaders(1);
        writer.update("keyFiltered");
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    cout << "testing an undecodable sample filter criteria... " << flush;
    {
        Topic<TestKey, string> topic(node, "sampleFilter");
        topic.setSampleFilter<TestKey>(
            "named",
            [](const TestKey& criteria)
            { return [criteria](const Sample<TestKey, string>& sample) { return sample.getKey() == criteria; }; });

        auto writer = makeSingleKeyWriter(topic, TestKey{"good"}, "", config);

        writer.waitForReaders(1);
        writer.update("filtered");
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
