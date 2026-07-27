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

    Topic<string, string> topic(node, "partialUpdateGap");
    Topic<string, int> barrier(node, "partialUpdateGapBarrier");
    topic.setUpdater<string>("append", [](string& value, const string& suffix) { value += suffix; });

    cout << "testing partial update after a reconnect across a history gap... " << flush;
    {
        // This writer keeps no history, so the samples it publishes while the session is down cannot be replayed when
        // the reader resumes from its last sample id. Only the per-key base, retained independently of history, can
        // bring the reader back up to date.
        WriterConfig config;
        config.sampleCount = 0;
        auto writer = makeSingleKeyWriter(topic, "key", "", config);
        writer.waitForReaders();
        writer.add("base");

        // Wait until the reader has consumed the base value, then close the session connection and publish while it is
        // down. The reader cannot learn of the closure before close completes, and reconnecting takes a full session
        // handshake, so both samples land in the gap.
        auto sample = makeSingleKeyReader(barrier, "ready").getNextUnread();
        auto connection = node.getSessionConnection(sample.getSession());
        test(connection);
        connection->close().get();
        writer.update("second");
        writer.partialUpdate<string>("append")("-tail");

        // Wait until the reader has consumed the value it resumed with, so this partial update is delivered live
        // rather than folded into the initialization batch.
        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "resumed").getNextUnread();
        writer.partialUpdate<string>("append")("-live");
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
