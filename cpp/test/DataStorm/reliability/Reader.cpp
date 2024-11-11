//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

#include <thread>

using namespace DataStorm;
using namespace std;

class Reader : public Test::TestHelper
{
public:
    Reader() : Test::TestHelper(false) {}

    void run(int, char**);
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
                    this_thread::sleep_for(chrono::milliseconds(200));
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
}

DEFINE_TEST(::Reader)
