// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

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
        Topic<string, string> topic(node, "string");
        auto barrier = makeSingleKeyWriter(topic, "barrier");
        barrier.waitForReaders();
        barrier.update("");
        auto reader = makeSingleKeyReader(topic, "element", "", config);
        test(reader.getNextUnread().getValue() == "add");
        test(reader.getNextUnread().getValue() == "update1");
        test(reader.getNextUnread().getValue() == "update2");
        barrier.update("");
        test(reader.getNextUnread().getValue() == "update3");
        barrier.update("");
        barrier.waitForNoReaders();
    }

    {
        Topic<string, int> topic(node, "int");
        auto reader = makeSingleKeyReader(topic, "element", "", config);
        for (int i = 0; i < 1000; ++i)
        {
            auto sample = reader.getNextUnread();
            if (sample.getValue() != i)
            {
                cerr << "unexpected sample: " << sample.getValue() << " expected:" << i << endl;
                test(false);
            }

            if ((i % 50) == 0)
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
        auto writer = makeSingleKeyWriter(topic, "barrier");
        writer.waitForReaders();
        writer.update(0);
        writer.waitForNoReaders();
    }

    {
        Topic<string, int> topic(node, "int2");
        auto reader = makeSingleKeyReader(topic, "element", "", config);
        string session;

        // Read 100 samples from the "element" key and close the connection.
        for (int i = 0; i < 100; ++i)
        {
            auto sample = reader.getNextUnread();
            if (sample.getValue() != i)
            {
                cerr << "unexpected sample: " << sample.getValue() << " expected:" << i << endl;
                test(false);
            }
            session = sample.getSession();
        }

        auto connection = node.getSessionConnection(session);
        test(connection);
        connection->close().get();

        // Send a sample to the writer on "reader_barrier" to let it know that the connection was closed.
        // The writer will read it after the session is reestablished.
        auto writerB = makeSingleKeyWriter(topic, "reader_barrier");
        writerB.waitForReaders();
        writerB.update(0);

        // Wait for the writer to acknowledge the sample send on "reader_barrier" and close the connection again.
        auto readerB = makeSingleKeyReader(topic, "writer_barrier");
        [[maybe_unused]] auto _ = readerB.getNextUnread();

        // Session was reestablished; close it again.
        connection = node.getSessionConnection(session);
        test(connection);
        connection->close().get();

        // Let the writer know the connection was closed again, and that it can proceed with the second batch of
        // samples.
        writerB.waitForReaders();
        writerB.update(0);

        for (int i = 0; i < 100; ++i)
        {
            auto sample = reader.getNextUnread();
            if (sample.getValue() != i + 100)
            {
                cerr << "unexpected sample: " << sample.getValue() << " expected:" << (i + 100) << endl;
                test(false);
            }
        }

        // Let the writer know we have processed all samples.
        writerB.waitForReaders();
        writerB.update(0);
        writerB.waitForNoReaders();
    }

    {
        Topic<string, int> topic(node, "anyKeyReconnect");
        Topic<string, int> barrier(node, "anyKeyReconnectBarrier");
        // A plain single-key reader against the any-key (filtered) writer: reconnect resumption is driven by the
        // writer being filtered, so a reader of any kind connected to it must resume rather than re-read history.
        auto reader = makeSingleKeyReader(topic, "k", "", config);
        auto writerB = makeSingleKeyWriter(barrier, "reader_barrier");

        string session;
        for (int i = 0; i < 100; ++i)
        {
            auto sample = reader.getNextUnread();
            if (sample.getValue() != i)
            {
                cerr << "unexpected sample: " << sample.getValue() << " expected:" << i << endl;
                test(false);
            }
            session = sample.getSession();
        }

        // Force a session reconnect while the writer retains its history.
        auto connection = node.getSessionConnection(session);
        test(connection);
        connection->close().get();

        // Tell the writer the connection closed (processed after reconnect); it then sends the second batch.
        writerB.waitForReaders();
        writerB.update(0);

        // After the reconnect the reader must continue from 100: the writer resumes from the reader's last received
        // sample rather than re-sending its whole retained queue (which would re-deliver 0..99).
        for (int i = 0; i < 100; ++i)
        {
            auto sample = reader.getNextUnread();
            if (sample.getValue() != i + 100)
            {
                cerr << "duplicate or rewound sample: " << sample.getValue() << " expected:" << (i + 100) << endl;
                test(false);
            }
        }

        writerB.waitForReaders();
        writerB.update(0);
    }

    {
        Topic<string, string> topic(node, "partialUpdateReconnect");
        Topic<string, int> barrier(node, "partialUpdateReconnectBarrier");
        topic.setUpdater<string>("append", [](string& value, const string& suffix) { value += suffix; });
        auto reader = makeSingleKeyReader(topic, "key", "", config);

        auto sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Add);
        test(sample.getValue() == "base");

        // Force a reconnect after the reader has established its partial-update base. Reconnection resumes from the
        // reader's last sample id, so the writer does not resend the full value; the reader must retain its base.
        auto connection = node.getSessionConnection(sample.getSession());
        test(connection);
        connection->close().get();

        // Signal the writer after the session has reconnected, then expect a partial update followed by a full update.
        // The trailing full update makes a dropped partial fail immediately instead of leaving this test blocked.
        auto ready = makeSingleKeyWriter(barrier, "ready");
        ready.waitForReaders();
        ready.update(0);

        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::PartialUpdate);
        test(sample.getValue() == "base-after-reconnect");

        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Update);
        test(sample.getValue() == "done");

        auto done = makeSingleKeyWriter(barrier, "done");
        done.waitForReaders();
        done.update(0);
    }
}

DEFINE_TEST(::Reader)
