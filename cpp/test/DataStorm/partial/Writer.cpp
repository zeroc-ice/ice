// Copyright (c) ZeroC, Inc.
#if defined(_MSC_VER)
#    pragma warning(4 : 4244)
#endif

#include "DataStorm/DataStorm.h"
#include "Test.h"
#include "TestHelper.h"

using namespace DataStorm;
using namespace std;
using namespace Test;

class Writer : public Test::TestHelper
{
public:
    Writer() : Test::TestHelper(false) {}

    void run(int, char**) override;
};

void ::Writer::run(int argc, char* argv[])
{
    Node node(argc, argv);

    Topic<string, StockPtr> topic(node, "topic");

    WriterConfig config;
    config.sampleCount = -1;
    config.clearHistory = ClearHistoryPolicy::Never;
    topic.setWriterDefaultConfig(config);

    topic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });

    cout << "testing partial update... " << flush;
    {
        auto writer = makeSingleKeyWriter(topic, "AAPL");
        writer.waitForReaders();
        writer.add(make_shared<Stock>(12.0f, 13.0f, 14.0f));
        writer.partialUpdate<float>("price")(15.0f);
        writer.partialUpdate<float>("price")(18);
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    Topic<string, StockPtr> multiTopic(node, "multiKeyTopic");
    multiTopic.setWriterDefaultConfig(config);
    multiTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });

    cout << "testing multi-key partial update... " << flush;
    {
        // Regression test for #5473: with a single per-element _last, a partial update on one key was resolved
        // against the most recent sample of a *different* key. Write GOOG right before the AAPL partial update so
        // that the AAPL partial's preceding sample (the single _last) belongs to GOOG.
        auto writer = makeMultiKeyWriter(multiTopic, {"AAPL", "GOOG"});
        writer.waitForReaders();
        writer.add("AAPL", make_shared<Stock>(12.0f, 13.0f, 14.0f));
        writer.add("GOOG", make_shared<Stock>(100.0f, 101.0f, 102.0f));
        writer.partialUpdate<float>("price")("AAPL", 15.0f);
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
