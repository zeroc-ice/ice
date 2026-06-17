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

    Topic<string, string> controlTopic(node, "control");
    auto control = makeSingleKeyReader(controlTopic, "control");

    // Used to tell the writer we have fully attached to its topic, so it only destroys the topic once the attach
    // is observed on both sides. Otherwise the writer can destroy (and detach us) before our waitForWriters() below
    // observes the writer, leaving this reader waiting for a writer that never comes back.
    Topic<string, string> syncTopic(node, "sync");
    auto sync = makeSingleKeyWriter(syncTopic, "sync");

    {
        Topic<string, string> topic(node, "topic");
        auto reader = makeSingleKeyReader(topic, "key");
        reader.waitForWriters(); // attach to the writer so the writer topic's _listenerCount is > 0
        sync.add("ready");       // tell the writer our attach is complete

        // Stay attached (keep 'reader' alive) until the writer reports it destroyed its topic while
        // we were still attached. This guarantees the writer hits the destroy-while-attached path.
        auto sample = control.getNextUnread();
        test(sample.getValue() == "done");
    } // 'reader' then 'topic' are destroyed here in the usual order
}

DEFINE_TEST(::Reader)
