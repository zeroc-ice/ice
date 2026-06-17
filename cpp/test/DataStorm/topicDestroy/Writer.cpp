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

    // A long-lived control topic (reversed roles) used to rendezvous with the reader. It has an
    // idiomatic lifetime, so it is unaffected by the topic destroyed mid-test below.
    Topic<string, string> controlTopic(node, "control");
    auto control = makeSingleKeyWriter(controlTopic, "control");

    // Reverse-direction topic the reader uses to confirm it has fully attached to our topic.
    Topic<string, string> syncTopic(node, "sync");
    auto sync = makeSingleKeyReader(syncTopic, "sync");

    cout << "testing topic destroyed while a writer is still attached to a peer... " << flush;
    {
        // Invert the usual destruction order: the writer element outlives its Topic handle. The
        // Topic is destroyed (inner scope) while 'writer' remains alive and still attached to the
        // reader. Before this fix (issue #5469) TopicI::destroy()'s disconnect went through
        // SessionI::runWithTopic, whose isDestroyed gate is always true on that path, so the
        // listener was never detached and TopicI::_listenerCount was left stale, tripping
        // assert(_listenerCount == 0) in TopicI::disconnect on debug builds.
        optional<SingleKeyWriter<string, string>> writer;
        {
            Topic<string, string> topic(node, "topic");
            writer.emplace(makeSingleKeyWriter(topic, "key"));
            writer->waitForReaders(); // the reader has attached: the topic's _listenerCount is > 0

            // Wait until the reader confirms its own attach completed, so the destroy below truly happens while
            // the reader is still attached (i.e. the reader's waitForWriters() has already returned, and won't be
            // left waiting for a writer we detached).
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
