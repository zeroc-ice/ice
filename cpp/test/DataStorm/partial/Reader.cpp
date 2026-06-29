// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "Test.h"
#include "TestHelper.h"

using namespace DataStorm;
using namespace std;
using namespace Test;

class Reader : public Test::TestHelper
{
public:
    Reader() : Test::TestHelper(false) {}

    void run(int, char**) override;
};

void ::Reader::run(int argc, char* argv[])
{
    Node node(argc, argv);

    Topic<string, StockPtr> topic(node, "topic");

    ReaderConfig config;
    config.sampleCount = -1;
    config.clearHistory = ClearHistoryPolicy::Never;
    topic.setReaderDefaultConfig(config);

    topic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });

    {
        auto reader = makeSingleKeyReader(topic, "AAPL");
        auto sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Add);
        test(sample.getValue()->price == 12.0f);

        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::PartialUpdate);
        test(sample.getUpdateTag() == "price");
        test(sample.getValue()->price == 15.0f);

        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::PartialUpdate);
        test(sample.getUpdateTag() == "price");
        test(sample.getValue()->price == 18.0f);

        // Late joining reader should still receive update events instead of partial updates
        auto reader2 = makeSingleKeyReader(topic, "AAPL");
        sample = reader2.getNextUnread();
        test(sample.getEvent() == SampleEvent::Add);
        test(sample.getValue()->price == 12.0f);

        sample = reader2.getNextUnread();
        test(sample.getEvent() == SampleEvent::PartialUpdate);
        test(sample.getValue()->price == 15.0f);

        sample = reader2.getNextUnread();
        test(sample.getEvent() == SampleEvent::PartialUpdate);
        test(sample.getValue()->price == 18.0f);

        // Late joining reader with limited sample count should receive one Update event and a PartialUpdate event
        auto reader3 = makeSingleKeyReader(topic, "AAPL", "", ReaderConfig(2));
        sample = reader3.getNextUnread();
        test(sample.getEvent() == SampleEvent::Update);
        test(sample.getValue()->price == 15.0f);

        sample = reader3.getNextUnread();
        test(sample.getEvent() == SampleEvent::PartialUpdate);
        test(sample.getValue()->price == 18.0f);
    }

    Topic<string, StockPtr> multiTopic(node, "multiKeyTopic");
    multiTopic.setReaderDefaultConfig(config);
    multiTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    {
        // The AAPL partial update only sets price; the fields it does not set (lastBid/lastAsk) must be carried
        // over from AAPL's own previous value, not from GOOG's.
        auto reader = makeMultiKeyReader(multiTopic, {"AAPL", "GOOG"});
        shared_ptr<Stock> aapl;
        for (int i = 0; i < 3; ++i)
        {
            auto sample = reader.getNextUnread();
            if (sample.getKey() == "AAPL" && sample.getEvent() == SampleEvent::PartialUpdate)
            {
                aapl = sample.getValue();
            }
        }
        test(aapl);
        test(aapl->price == 15.0f);
        test(aapl->lastBid == 13.0f); // AAPL's own bid, not GOOG's 101
        test(aapl->lastAsk == 14.0f); // AAPL's own ask, not GOOG's 102
    }

    // A persistent multi-key reader attaches to writer 1 under both keys; writer 1's detach leaves a stale entry
    // in the reader's per-key subscriber map, and writer 2's attach drives SessionI::getLastIds over it, which
    // must not dereference null.
    Topic<string, int> detachTopic(node, "multiKeyDetach");
    detachTopic.setReaderDefaultConfig(config);
    Topic<string, int> detachBarrier(node, "multiKeyDetachBarrier");
    {
        auto reader = makeMultiKeyReader(detachTopic, {"k1", "k2"});
        auto sample = reader.getNextUnread(); // from writer 1
        test(sample.getValue() == 1);

        // Wait until writer 1's detach has been processed (this is what leaves the stale per-key entry), then
        // tell the writer it can create writer 2.
        reader.waitForNoWriters();
        auto barrier = makeSingleKeyWriter(detachBarrier, "barrier");
        barrier.waitForReaders();
        barrier.update(0);

        sample = reader.getNextUnread(); // from writer 2: attach runs getLastIds over the stale entry
        test(sample.getValue() == 2);
    }
}

DEFINE_TEST(::Reader)
