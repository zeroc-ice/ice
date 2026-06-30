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

    Topic<string, int> topic(node, "topic");
    auto writer = makeSingleKeyWriter(topic, "key", "", config);

    // Wait for both subscribers to attach through the relay, then publish a sample to them.
    writer.waitForReaders(2);
    writer.update(42);
    cout << "writer published" << endl;

    // Stay alive until the test terminates this process; that drops the relay's connection to this publisher and
    // the subscribers are notified that there are no more writers.
    writer.waitForNoReaders();
    cout << "writer completed" << endl;
}

DEFINE_TEST(::Writer)
