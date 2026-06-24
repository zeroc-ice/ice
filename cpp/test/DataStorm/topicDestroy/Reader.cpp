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

    Topic<string, string> controlTopic(node, "control");
    auto control = makeSingleKeyReader(controlTopic, "control");

    // Tells the writer this reader has fully attached, so it only destroys its topic once the attach is observed
    // on both sides (otherwise the writer could detach us before our waitForWriters() below returns).
    Topic<string, string> syncTopic(node, "sync");
    auto sync = makeSingleKeyWriter(syncTopic, "sync");

    Topic<string, string> topic(node, "topic");
    auto reader = makeSingleKeyReader(topic, "key");
    reader.waitForWriters(); // attach to the writer
    sync.add("ready");       // tell the writer our attach is complete

    // Stay attached until the writer reports it destroyed its topic while we were still attached.
    auto sample = control.getNextUnread();
    test(sample.getValue() == "done");
}

DEFINE_TEST(::Reader)
