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

    // Publish a batch of samples to a topic's key, follow by two consecutive session recovery events without writer
    // activity on the given key.
    // Then send a second batch of samples to the same topic's key and ensure the reader continue reading from when it
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

        // Wait for a second control sample from the reader indicating the second session closure. The writer process
        // this sample after the second session reestablishment.
        sample = readerB.getNextUnread();

        // Session has been reestablish twice without activity in "element" key. Send the second batch of samples.
        for (int i = 0; i < 100; ++i)
        {
            writer.update(i + 100);
        }
        sample = readerB.getNextUnread();
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
