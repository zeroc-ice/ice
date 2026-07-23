// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "Test.h"
#include "TestHelper.h"

#include <future>

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

    // A sampleCount=0 reader keeps no history but must still resolve a partial update against the previous full
    // sample of the same key.
    Topic<string, StockPtr> noHistoryTopic(node, "noHistoryTopic");
    noHistoryTopic.setReaderDefaultConfig(config);
    noHistoryTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> noHistoryBarrier(node, "noHistoryBarrier");
    {
        ReaderConfig noHistory;
        noHistory.sampleCount = 0;
        auto reader = makeSingleKeyReader(noHistoryTopic, "AAPL", "", noHistory);

        // A sampleCount=0 reader keeps no history, so getNextUnread would never return; observe samples through the
        // update callback instead. It fires for the Add (which establishes the base) and then the partial update.
        promise<shared_ptr<Stock>> partial;
        reader.onSamples(
            [](const vector<Sample<string, StockPtr>>&) {},
            [&partial](const Sample<string, StockPtr>& sample)
            {
                if (sample.getEvent() == SampleEvent::PartialUpdate)
                {
                    partial.set_value(sample.getValue());
                }
            });

        // Tell the writer the callback is installed so it can publish.
        auto barrier = makeSingleKeyWriter(noHistoryBarrier, "barrier");
        barrier.waitForReaders();
        barrier.update(0);

        auto aapl = partial.get_future().get();
        test(aapl);
        test(aapl->price == 15.0f);   // the partial update
        test(aapl->lastBid == 13.0f); // carried over from the base, not a default-constructed 0
        test(aapl->lastAsk == 14.0f);
    }

    // A reader that joins after a no-history (sampleCount == 0) writer's initial value is bootstrapped with the
    // writer's current per-key base value, so a subsequent partial update resolves against it instead of crashing on
    // a null base.
    Topic<string, StockPtr> lateJoinTopic(node, "lateJoinTopic");
    lateJoinTopic.setReaderDefaultConfig(config);
    lateJoinTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> lateJoinBarrier(node, "lateJoinBarrier");
    {
        // Wait until the writer has published its initial value, then attach as a late joiner.
        [[maybe_unused]] auto _ = makeSingleKeyReader(lateJoinBarrier, "barrier").getNextUnread();

        auto reader = makeSingleKeyReader(lateJoinTopic, "AAPL");

        // The reader receives the bootstrapped base, then the resolved partial update, then a later full value.
        shared_ptr<Stock> resolved;
        for (int i = 0; i < 3; ++i)
        {
            auto sample = reader.getNextUnread();
            if (sample.getEvent() == SampleEvent::PartialUpdate)
            {
                resolved = sample.getValue();
            }
        }
        test(resolved);
        test(resolved->price == 15.0f);   // the partial update
        test(resolved->lastBid == 13.0f); // carried from the bootstrapped base, not a default-constructed 0
        test(resolved->lastAsk == 14.0f);
    }

    // Same as above, but the writer keeps history with a short sampleLifetime; the full sample ages out before the
    // reader joins, so the reader must still be bootstrapped from the (lifetime-independent) per-key base.
    Topic<string, StockPtr> lifetimeTopic(node, "lifetimeTopic");
    lifetimeTopic.setReaderDefaultConfig(config);
    lifetimeTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> lifetimeBarrier(node, "lifetimeBarrier");
    {
        [[maybe_unused]] auto _ = makeSingleKeyReader(lifetimeBarrier, "barrier").getNextUnread();

        auto reader = makeSingleKeyReader(lifetimeTopic, "AAPL");

        shared_ptr<Stock> resolved;
        for (int i = 0; i < 3; ++i)
        {
            auto sample = reader.getNextUnread();
            if (sample.getEvent() == SampleEvent::PartialUpdate)
            {
                resolved = sample.getValue();
            }
        }
        test(resolved);
        test(resolved->price == 15.0f);
        test(resolved->lastBid == 13.0f); // carried from the aged-out-but-still-current base
        test(resolved->lastAsk == 14.0f);
    }

    // A reader that keeps no history (sampleCount == 0) and joins after the writer's add is bootstrapped with the base
    // value, so the subsequent partial update resolves against it. A sampleCount=0 reader keeps no history, so it is
    // observed through the onSamples callback.
    Topic<string, StockPtr> lateReaderTopic(node, "lateReaderTopic");
    lateReaderTopic.setReaderDefaultConfig(config);
    lateReaderTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> lateReaderJoinBarrier(node, "lateReaderJoinBarrier");
    Topic<string, int> lateReaderReadyBarrier(node, "lateReaderReadyBarrier");
    {
        // Wait until the add is published, then join late with sampleCount=0.
        [[maybe_unused]] auto _ = makeSingleKeyReader(lateReaderJoinBarrier, "barrier").getNextUnread();

        ReaderConfig noHistory;
        noHistory.sampleCount = 0;
        auto reader = makeSingleKeyReader(lateReaderTopic, "AAPL", "", noHistory);

        promise<shared_ptr<Stock>> partial;
        reader.onSamples(
            [](const vector<Sample<string, StockPtr>>&) {},
            [&partial](const Sample<string, StockPtr>& sample)
            {
                if (sample.getEvent() == SampleEvent::PartialUpdate)
                {
                    partial.set_value(sample.getValue());
                }
            });

        // Tell the writer the callback is installed.
        auto readyBarrier = makeSingleKeyWriter(lateReaderReadyBarrier, "barrier");
        readyBarrier.waitForReaders();
        readyBarrier.update(0);

        auto aapl = partial.get_future().get();
        test(aapl);
        test(aapl->price == 15.0f);
        test(aapl->lastBid == 13.0f); // resolved against the bootstrapped base
        test(aapl->lastAsk == 14.0f);
    }

    // A late any-key reader with the default history policy fetches only the newest sample for all keys, so AAPL's
    // full value (trimmed from the writer's history) must be bootstrapped from the per-key base for the partial
    // update to resolve rather than crash on a null base.
    Topic<string, StockPtr> trimTopic(node, "trimHistoryTopic"); // default config: ClearHistory=OnAll
    trimTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> trimBarrier(node, "trimHistoryBarrier");
    {
        [[maybe_unused]] auto _ = makeSingleKeyReader(trimBarrier, "barrier").getNextUnread();

        auto reader = makeAnyKeyReader(trimTopic);
        shared_ptr<Stock> aapl;
        while (!aapl)
        {
            auto sample = reader.getNextUnread();
            if (sample.getKey() == "AAPL" && sample.getEvent() == SampleEvent::PartialUpdate)
            {
                aapl = sample.getValue();
            }
        }
        test(aapl->price == 15.0f);
        test(aapl->lastBid == 13.0f); // carried from AAPL's own base, bootstrapped after the history trim
        test(aapl->lastAsk == 14.0f);
    }

    // A reader with a limited sampleCount joins a writer that has more distinct keys (MSFT, AAPL, GOOG) than the
    // reader's sampleCount, with GOOG carrying extra history. Capping the init batch to sampleCount must keep a base
    // for every key, including MSFT (the oldest, which a naive "newest N samples" cap drops), so a later partial
    // update on MSFT resolves rather than crashing on a null base.
    Topic<string, StockPtr> capTopic(node, "capTopic");
    capTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> capBarrier(node, "capBarrier");
    {
        [[maybe_unused]] auto _ = makeSingleKeyReader(capBarrier, "barrier").getNextUnread();

        ReaderConfig limited;
        limited.sampleCount = 2;
        limited.clearHistory = ClearHistoryPolicy::Never;
        auto reader = makeAnyKeyReader(capTopic, "", limited);
        shared_ptr<Stock> msft;
        while (!msft)
        {
            auto sample = reader.getNextUnread();
            if (sample.getKey() == "MSFT" && sample.getEvent() == SampleEvent::PartialUpdate)
            {
                msft = sample.getValue();
            }
        }
        test(msft->price == 55.0f);
        test(msft->lastBid == 51.0f); // MSFT's base survived the cap despite the key count exceeding sampleCount
        test(msft->lastAsk == 52.0f);
    }

    // The reader-side updater throws on one of the initialization samples: only that sample is dropped, the other
    // initialization samples are delivered, and the following partial update resolves against the last successfully
    // applied value.
    Topic<string, StockPtr> throwTopic(node, "throwingUpdater");
    throwTopic.setReaderDefaultConfig(config); // keep full history, so the samples arrive unmerged
    throwTopic.setUpdater<float>(
        "price",
        [](StockPtr& stock, float price)
        {
            if (price < 0)
            {
                throw std::runtime_error("negative price");
            }
            stock->price = price;
        });
    Topic<string, int> throwBarrier(node, "throwingUpdaterBarrier");
    {
        [[maybe_unused]] auto _ = makeSingleKeyReader(throwBarrier, "barrier").getNextUnread();

        auto reader = makeSingleKeyReader(throwTopic, "AAPL");

        auto sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Add);
        test(sample.getValue()->price == 12.0f);

        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::PartialUpdate);
        test(sample.getValue()->price == 15.0f);   // the throwing sample was dropped...
        test(sample.getValue()->lastBid == 13.0f); // ...and the update resolved against the Add sample

        // Tell the writer the samples were verified.
        auto done = makeSingleKeyWriter(throwBarrier, "done");
        done.waitForReaders();
        done.update(0);
    }

    // A partial update requires the key to have a current value: after a remove, publishing a partial update throws
    // on the writer and nothing is published; the next full value makes the key updatable again.
    Topic<string, StockPtr> removeTopic(node, "removeTopic");
    removeTopic.setReaderDefaultConfig(config);
    removeTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    {
        auto reader = makeSingleKeyReader(removeTopic, "AAPL");

        auto sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Add);
        test(sample.getValue()->price == 12.0f);

        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Remove);

        // The discarded partial update was never published: the next sample is the full update.
        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Update);
        test(sample.getValue()->price == 20.0f);
    }

    // Two writers publish the same key: writer 2's partial update reaches the reader after writer 1's remove
    // cleared the key's value. The reader discards the partial update and resynchronizes on writer 2's next full
    // value.
    Topic<string, StockPtr> twoWritersTopic(node, "twoWritersTopic");
    twoWritersTopic.setReaderDefaultConfig(config);
    twoWritersTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    {
        auto reader = makeSingleKeyReader(twoWritersTopic, "AAPL");

        // Consume until writer 2's resynchronizing full value; the partial update published after the remove must
        // never surface.
        bool sawRemove = false;
        shared_ptr<Stock> resync;
        while (!resync)
        {
            auto sample = reader.getNextUnread();
            test(sample.getEvent() != SampleEvent::PartialUpdate);
            if (sample.getEvent() == SampleEvent::Remove)
            {
                sawRemove = true;
            }
            else if (sample.getValue()->price == 200.0f)
            {
                resync = sample.getValue();
            }
        }
        test(sawRemove);
        test(resync->lastBid == 201.0f); // writer 2's full value, not a merge against a stale base
    }

    // A late-joining reader initializes across a remove. It receives the add, the remove, the re-add, and the partial
    // update as initialization samples; the remove clears the key's base within the init batch and the partial
    // resolves against the re-added value, not the pre-remove value or a default-constructed one.
    Topic<string, StockPtr> initRemoveTopic(node, "initRemoveTopic");
    initRemoveTopic.setReaderDefaultConfig(config);
    initRemoveTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> initRemoveBarrier(node, "initRemoveBarrier");
    {
        // Wait until the writer has published all four samples, then attach as a late joiner.
        [[maybe_unused]] auto _ = makeSingleKeyReader(initRemoveBarrier, "barrier").getNextUnread();

        auto reader = makeSingleKeyReader(initRemoveTopic, "AAPL");

        auto sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Add);
        test(sample.getValue()->price == 12.0f);

        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Remove);

        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Add);
        test(sample.getValue()->price == 20.0f);

        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::PartialUpdate);
        test(sample.getValue()->price == 25.0f);   // resolved against the re-added value
        test(sample.getValue()->lastBid == 21.0f); // carried from the re-added value, not the pre-remove value
        test(sample.getValue()->lastAsk == 22.0f);

        // Tell the writer the samples were verified.
        auto done = makeSingleKeyWriter(initRemoveBarrier, "done");
        done.waitForReaders();
        done.update(0);
    }
}

DEFINE_TEST(::Reader)
