// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

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

    ReaderConfig config;
    config.clearHistory = ClearHistoryPolicy::Never;

    {
        Topic<string, int> topic(node, "writerActive");
        auto reader = makeSingleKeyReader(topic, "element", "", config);

        // The writer publishes 0..999 at a steady pace. We read the whole sequence and force a session reconnect
        // every 50 samples -- while the writer is still publishing -- so each reconnect overlaps live `s()` sample
        // forwards with the resync. Every sample must arrive exactly once and in order: a regression in the
        // writer-active resync window would surface here as a duplicate, a gap, or an out-of-order value.
        for (int i = 0; i < 1000; ++i)
        {
            auto sample = reader.getNextUnread();
            if (sample.getValue() != i)
            {
                cerr << "unexpected sample: " << sample.getValue() << " expected: " << i << endl;
                test(false);
            }

            if (i > 0 && (i % 50) == 0)
            {
                auto connection = node.getSessionConnection(sample.getSession());
                while (!connection)
                {
                    this_thread::sleep_for(chrono::milliseconds(10));
                    connection = node.getSessionConnection(sample.getSession());
                }
                connection->close().get();
            }
        }

        // Signal the writer that the full sequence was received exactly once.
        auto barrier = makeSingleKeyWriter(topic, "barrier");
        barrier.waitForReaders();
        barrier.update(0);
        barrier.waitForNoReaders();
    }
}

DEFINE_TEST(::Reader)
