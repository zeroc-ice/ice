// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

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

    // The reader-app connects to the tail of the relay chain. Creating this reader announces the
    // "topic" topic reader before the full relay path exists.
    Topic<string, int> topic(node, "topic");
    auto reader = makeSingleKeyReader(topic, "key", "", config);

    // Block until the writer's sample is received across the relay chain. This only completes once
    // discovery propagates across the relay path that is completed after both announcements were
    // received - the failure mode this test reproduces.
    auto sample = reader.getNextUnread();
    test(sample.getValue() == 42);

    cout << "reader completed" << endl;
}

DEFINE_TEST(::Reader)
