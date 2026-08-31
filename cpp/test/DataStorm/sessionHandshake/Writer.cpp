// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

using namespace std;

// The node under test. It only has to offer a topic writer for the peer to announce a reader for; the peer drives
// every session creation exchange from there.
class Writer : public Test::TestHelper
{
public:
    Writer() : Test::TestHelper(false) {}

    void run(int, char**) override;
};

void ::Writer::run(int argc, char* argv[])
{
    DataStorm::Node node(argc, argv);
    DataStorm::Topic<string, string> topic(node, "string");
    auto writer = DataStorm::makeSingleKeyWriter(topic, "element");
    writer.update("value");

    // The peer decides when the test is over; this node runs until the test framework stops it.
    node.getCommunicator()->waitForShutdown();
}

DEFINE_TEST(::Writer)
