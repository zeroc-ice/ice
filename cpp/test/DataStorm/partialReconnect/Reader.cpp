// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

using namespace DataStorm;
using namespace std;

class Reader : public Test::TestHelper
{
public:
    Reader() : Test::TestHelper(false) {}

    void run(int, char**) override;
};

void ::Reader::run(int argc, char* argv[])
{
    Node node(argc, argv);

    Topic<string, string> topic(node, "partialUpdateGap");
    Topic<string, int> barrier(node, "partialUpdateGapBarrier");
    topic.setUpdater<string>("append", [](string& value, const string& suffix) { value += suffix; });

    {
        ReaderConfig config;
        config.clearHistory = ClearHistoryPolicy::Never;
        auto reader = makeSingleKeyReader(topic, "key", "", config);

        auto sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Add);
        test(sample.getValue() == "base");

        // Tell the writer the base is established. It closes the session connection and, while the reader is
        // disconnected, publishes a full value followed by a partial update, neither of which reaches the reader.
        auto ready = makeSingleKeyWriter(barrier, "ready");
        ready.waitForReaders();
        ready.update(0);

        // Both samples arrive as a single full update carrying the resolved value. Replaying the writer's history
        // would instead deliver "second" as an update of its own, followed by the partial update.
        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Update);
        test(sample.getValue() == "second-tail");

        // A partial update published after the reader resumed resolves against the value it resumed with.
        auto resumed = makeSingleKeyWriter(barrier, "resumed");
        resumed.waitForReaders();
        resumed.update(0);

        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::PartialUpdate);
        test(sample.getValue() == "second-tail-live");
    }
}

DEFINE_TEST(::Reader)
