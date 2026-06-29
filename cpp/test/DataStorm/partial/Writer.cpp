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
        // Publish AAPL's full value, then GOOG, then a partial update on AAPL. The partial update must be merged
        // onto AAPL's own previous value, not onto GOOG's (the most recently published sample).
        auto writer = makeMultiKeyWriter(multiTopic, {"AAPL", "GOOG"});
        writer.waitForReaders();
        writer.add("AAPL", make_shared<Stock>(12.0f, 13.0f, 14.0f));
        writer.add("GOOG", make_shared<Stock>(100.0f, 101.0f, 102.0f));
        writer.partialUpdate<float>("price")("AAPL", 15.0f);
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // Destroying a multi-key writer leaves a stale entry in the reader's per-key subscriber map
    // (DataElementI::detachKey detaches every key with the element's first key id). A later attach then drives
    // SessionI::getLastIds over the stale entry, which must not crash the reader.
    Topic<string, int> detachTopic(node, "multiKeyDetach");
    detachTopic.setWriterDefaultConfig(config);
    Topic<string, int> detachBarrier(node, "multiKeyDetachBarrier");
    cout << "testing multi-key writer detach then reattach... " << flush;
    {
        {
            auto writer = makeMultiKeyWriter(detachTopic, {"k1", "k2"});
            writer.waitForReaders();
            writer.add("k1", 1);
        } // writer destroyed here -> detachElements -> stale per-key entry on the reader

        // Wait for the reader to confirm it has processed the detach before creating the second writer, so the
        // second attach is guaranteed to run getLastIds over the stale entry.
        [[maybe_unused]] auto _ = makeSingleKeyReader(detachBarrier, "barrier").getNextUnread();

        auto writer = makeMultiKeyWriter(detachTopic, {"k1", "k2"});
        writer.waitForReaders();
        writer.add("k2", 2);
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
