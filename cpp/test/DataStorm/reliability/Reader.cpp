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
}

DEFINE_TEST(::Reader)
