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

    Topic<string, string> topic(node, "topic");
    Topic<string, int> barrier(node, "barrier");

    // A sample-filtered reader is addressed under a session facet of its own, which has to survive the relay hop.
    auto reader = makeSingleKeyReader(topic, "elem", Filter<string>("contains", "a"), "", config);
    auto barrierReader = makeSingleKeyReader(barrier, "b", "", config);

    // The writer publishes the barrier value last, on the same session and therefore the same connections, and a
    // node dispatches each connection's requests one at a time — the relay's adapter and these applications'
    // client thread pool are both serialized. Once the barrier value is here the filtered samples have been
    // delivered too, if they were delivered at all.
    [[maybe_unused]] auto _ = barrierReader.getNextUnread();

    // The writer published "a", "b" and "ac"; the filter matches the two that contain "a".
    vector<string> values;
    for (const auto& sample : reader.getAllUnread())
    {
        values.push_back(sample.getValue());
    }
    test(values == vector<string>({"a", "ac"}));

    cout << "reader completed" << endl;
}

DEFINE_TEST(::Reader)
