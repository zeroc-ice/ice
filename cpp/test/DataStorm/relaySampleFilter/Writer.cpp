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

    Topic<string, string> topic(node, "topic");
    topic.setSampleFilter<string>(
        "contains",
        [](const string& substring)
        {
            return [substring](const Sample<string, string>& sample)
            { return sample.getValue().find(substring) != string::npos; };
        });

    // A second topic with no sample filter. Its value is published last, so the reader can tell "the filtered
    // samples have not arrived yet" from "they were never delivered".
    Topic<string, int> barrier(node, "barrier");

    auto writer = makeSingleKeyWriter(topic, "elem", "", config);
    auto barrierWriter = makeSingleKeyWriter(barrier, "b", "", config);

    writer.waitForReaders(1);
    barrierWriter.waitForReaders(1);

    // Published after the reader attached, so these travel the live forwarding path across the relay rather than
    // the reader's initialization batch, which is routed to the reader element rather than to its facet.
    writer.update("a");  // matches the reader's sample filter
    writer.update("b");  // does not
    writer.update("ac"); // matches
    barrierWriter.update(1);
    cout << "writer published" << endl;

    writer.waitForNoReaders();
    cout << "writer completed" << endl;
}

DEFINE_TEST(::Writer)
