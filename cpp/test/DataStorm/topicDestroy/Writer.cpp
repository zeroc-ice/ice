// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

#include <optional>

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

    // A long-lived control topic (reversed roles) used to rendezvous with the reader; it outlives the test and
    // is unaffected by the topic destroyed mid-test below.
    Topic<string, string> controlTopic(node, "control");
    auto control = makeSingleKeyWriter(controlTopic, "control");

    // Reverse-direction topic the reader uses to confirm it has fully attached to our topic.
    Topic<string, string> syncTopic(node, "sync");
    auto sync = makeSingleKeyReader(syncTopic, "sync");

    cout << "testing topic destroyed while a writer is still attached to a peer... " << flush;
    {
        // Destroy the topic while its writer element is still alive and attached to the reader: 'writer'
        // outlives its Topic handle, so the Topic is destroyed (inner scope) before 'writer', inverting the
        // usual destruction order.
        optional<SingleKeyWriter<string, string>> writer;
        {
            Topic<string, string> topic(node, "topic");
            writer.emplace(makeSingleKeyWriter(topic, "key"));
            writer->waitForReaders(); // the reader has attached

            // Wait until the reader confirms its attach completed, so the destroy below happens while the
            // reader is still attached.
            test(sync.getNextUnread().getValue() == "ready");

            control.waitForReaders(); // the control session is established
        } // 'topic' is destroyed here while 'writer' is still alive and attached to the reader
    } // 'writer' is destroyed here, detaching from its already-destroyed topic
    cout << "ok" << endl;

    // Tell the reader the destroy completed, then keep the session up until the reader is done.
    control.add("done");
    control.waitForNoReaders();
}

DEFINE_TEST(::Writer)
