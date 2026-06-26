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

    // Subscribe to the publisher's topic, relayed through the relay node. This creates this node's first
    // subscriber session; every fresh subscriber node assigns it the same identity {"1","s"}.
    Topic<string, int> topic(node, "topic");
    auto reader = makeSingleKeyReader(topic, "key", "", config);

    // Read the publisher's sample to confirm we are attached to the publisher through the relay.
    auto sample = reader.getNextUnread();
    test(sample.getValue() == 42);
    cout << "reader attached" << endl;

    // After the relay loses its connection to the publisher, every subscriber must observe the writer
    // disconnecting. If the relay dropped this subscriber's session (its identity collided with another
    // subscriber's and was overwritten in the relay's session map), this subscriber is never told and
    // waitForNoWriters blocks forever.
    reader.waitForNoWriters();
    cout << "reader saw no writers" << endl;
}

DEFINE_TEST(::Reader)
