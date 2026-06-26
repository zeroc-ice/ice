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

    // Wait for both subscribers to attach through the relay, so both subscriber sessions are registered with
    // the relay's node session for this publisher -- where their colliding identities overwrite each other.
    writer.waitForReaders(2);
    writer.update(42);
    cout << "writer published" << endl;

    // Stay alive until the test terminates this process. Terminating it drops the relay's connection to this
    // publisher, which is what triggers the disconnect notifications the subscribers are waiting for.
    writer.waitForNoReaders();
    cout << "writer completed" << endl;
}

DEFINE_TEST(::Writer)
