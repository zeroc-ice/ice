// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

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
    config.clearHistory = ClearHistoryPolicy::Never;

    // The writer-app connects to the head of the relay chain. Creating this writer announces the
    // "topic" topic writer before the full relay path exists.
    Topic<string, int> topic(node, "topic");
    auto writer = makeSingleKeyWriter(topic, "key", "", config);

    // Block until the reader is discovered across the relay chain. The reader's announcement was
    // received by the tail relay before the relay path was complete, so this only completes if the
    // relays replay the announcements they received earlier across the new node sessions. Without
    // that replay, discovery deadlocks here.
    writer.waitForReaders();

    writer.update(42);

    // Wait until the reader has read the sample and disconnected before exiting, so the sample is
    // not lost to a premature shutdown.
    writer.waitForNoReaders();

    cout << "writer completed" << endl;
}

DEFINE_TEST(::Writer)
