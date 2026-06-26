// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

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

    cout << "testing reader reconnect while the writer is actively publishing... " << flush;
    {
        Topic<string, int> topic(node, "writerActive");
        auto writer = makeSingleKeyWriter(topic, "element", "", config);
        writer.waitForReaders();

        // Publish a known sequence at a steady pace. Unlike the reliability test (which publishes the whole batch
        // up front and then stays idle while the reader reconnects), the pacing keeps the writer publishing *live*
        // while the reader forces its reconnections, so live `s()` sample forwards race the session resync.
        for (int i = 0; i < 1000; ++i)
        {
            writer.update(i);
            this_thread::sleep_for(chrono::milliseconds(1));
        }

        // Wait for the reader to confirm it received the whole sequence exactly once and in order.
        [[maybe_unused]] auto _ = makeSingleKeyReader(topic, "barrier").getNextUnread();
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
