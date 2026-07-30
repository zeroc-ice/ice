// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

#include <condition_variable>
#include <mutex>

using namespace DataStorm;
using namespace std;

// Records that the writer's reader disconnected and then reconnected. The disconnect edge is latched, so a reconnect
// that completes before the writer starts waiting is not missed.
struct ReconnectGate
{
    std::mutex mutex;
    std::condition_variable condition;
    bool disconnected{false};
    bool reconnected{false};
};

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

    cout << "testing writer connection closure... " << flush;
    {
        Topic<string, string> topic(node, "string");
        auto writer = makeSingleKeyWriter(topic, "element", "", config);
        writer.add("add");
        writer.update("update1");
        auto barrier = makeSingleKeyReader(topic, "barrier");
        auto sample = barrier.getNextUnread();
        writer.waitForReaders();
        auto connection = node.getSessionConnection(sample.getSession());
        test(connection);
        connection->close().get();
        writer.update("update2");
        sample = barrier.getNextUnread();
        writer.update("update3");
        sample = barrier.getNextUnread();
    }
    cout << "ok" << endl;

    cout << "testing reader connection closure... " << flush;
    {
        Topic<string, int> topic(node, "int");
        auto writer = makeSingleKeyWriter(topic, "element", "", config);
        writer.waitForReaders();
        for (int i = 0; i < 1000; ++i)
        {
            writer.update(i);
        }
        [[maybe_unused]] auto _ = makeSingleKeyReader(topic, "barrier").getNextUnread();
    }
    cout << "ok" << endl;

    // Publish a batch of samples to a topic's key, followed by two consecutive session recovery events without writer
    // activity on the given key.
    // Then send a second batch of samples to the same topic's key and ensure the reader continues reading from when it
    // left off.
    cout << "testing reader multiple connection closure without writer activity... " << flush;
    {
        Topic<string, int> topic(node, "int2");
        auto writer = makeSingleKeyWriter(topic, "element", "", config);
        writer.waitForReaders();
        for (int i = 0; i < 100; ++i)
        {
            writer.update(i);
        }

        auto readerB = makeSingleKeyReader(topic, "reader_barrier");

        // A control sample sent by the reader to let the writer know the connection was closed. The writer processes
        // this sample after the first session reestablishment.
        auto sample = readerB.getNextUnread();

        // Send a control sample to let the reader know session was reestablished.
        auto writerB = makeSingleKeyWriter(topic, "writer_barrier");
        writerB.update(0);

        // Wait for a second control sample from the reader indicating the second session closure. The writer processes
        // this sample after the second session reestablishment.
        sample = readerB.getNextUnread();

        // The session has been reestablished twice without activity on the "element" key. Send the second batch of
        // samples.
        for (int i = 0; i < 100; ++i)
        {
            writer.update(i + 100);
        }
        sample = readerB.getNextUnread();
    }
    cout << "ok" << endl;

    cout << "testing reconnect with an any-key writer without duplicate samples... " << flush;
    {
        Topic<string, int> topic(node, "anyKeyReconnect");
        Topic<string, int> barrier(node, "anyKeyReconnectBarrier");
        auto writer = makeAnyKeyWriter(topic, "", config);
        writer.waitForReaders();
        for (int i = 0; i < 100; ++i)
        {
            writer.update("k", i);
        }

        // Wait for the reader to signal it read the batch and closed the connection.
        auto readerB = makeSingleKeyReader(barrier, "reader_barrier");
        auto sample = readerB.getNextUnread();

        // The session has been reestablished; send a second batch. The reader must continue from 100, not
        // re-receive the retained 0..99.
        for (int i = 0; i < 100; ++i)
        {
            writer.update("k", i + 100);
        }
        sample = readerB.getNextUnread();
    }
    cout << "ok" << endl;

    cout << "testing partial update after reconnect... " << flush;
    {
        Topic<string, string> topic(node, "partialUpdateReconnect");
        Topic<string, int> barrier(node, "partialUpdateReconnectBarrier");
        topic.setUpdater<string>("append", [](string& value, const string& suffix) { value += suffix; });
        auto writer = makeSingleKeyWriter(topic, "key", "", config);

        // Latch the reader's disconnect and the reconnect that follows it. waitForReaders only inspects the current
        // listener count, so on its own it can return on the pre-disconnect attachment while the reader's detach is
        // still on its way; with a relay between the two nodes the reader's barrier signal can arrive first. A sample
        // published then is not delivered live, and the reattach folds it into an initialization batch, where a
        // leading partial update is resolved into a full one. The connect callback is queued by the attach that
        // computes that batch while it holds the topic mutex, and publishing takes the same mutex, so a sample
        // published after the callback cannot overtake the batch.
        auto gate = make_shared<ReconnectGate>();
        writer.onConnectedReaders(
            [](const vector<string>&) {},
            [gate](CallbackReason reason, const string&)
            {
                lock_guard lock{gate->mutex};
                if (reason == CallbackReason::Disconnect)
                {
                    gate->disconnected = true;
                }
                else if (gate->disconnected)
                {
                    gate->reconnected = true;
                    gate->condition.notify_all();
                }
            });

        writer.waitForReaders();
        writer.add("base");

        // Wait until the reader consumes the full value, closes the connection, and reconnects. The reconnect resumes
        // after the add, so this partial update must resolve against the reader's retained pre-disconnect base.
        [[maybe_unused]] auto ready = makeSingleKeyReader(barrier, "ready").getNextUnread();
        {
            // The bound only exists to fail with an assertion rather than block forever, so it has to sit outside the
            // window in which DataStorm still recovers on its own: six retries backing off from 500ms, doubled again
            // for a peer whose endpoints are unknown, is 64 seconds. That puts it above the 60 second no-output
            // watchdog the test driver applies to local processes (scripts/Util.py), and this wait is silent, so a
            // genuine failure is reported there as a hanging process before the assertion fires. In CI the driver
            // waits 300 seconds and the assertion comes first.
            unique_lock lock{gate->mutex};
            test(gate->condition.wait_for(lock, chrono::seconds(90), [&gate] { return gate->reconnected; }));
        }

        // The latch records that a reader reconnected, not that one is attached now, so keep the level check too.
        writer.waitForReaders();
        writer.partialUpdate<string>("append")("-after-reconnect");
        writer.update("done");

        // Keep the writer alive until the reader verifies both post-reconnect samples.
        [[maybe_unused]] auto done = makeSingleKeyReader(barrier, "done").getNextUnread();
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
