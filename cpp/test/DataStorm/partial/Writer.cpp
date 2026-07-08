// Copyright (c) ZeroC, Inc.
#if defined(_MSC_VER)
#    pragma warning(4 : 4244)
#endif

#include "DataStorm/DataStorm.h"
#include "Test.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

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

    // A reader that keeps no history (sampleCount == 0) must still resolve a partial update against the previous
    // full sample of the same key, because the per-key base is recorded independently of history retention.
    Topic<string, StockPtr> noHistoryTopic(node, "noHistoryTopic");
    noHistoryTopic.setWriterDefaultConfig(config);
    noHistoryTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> noHistoryBarrier(node, "noHistoryBarrier");
    cout << "testing partial update delivered to a sampleCount=0 reader... " << flush;
    {
        auto writer = makeSingleKeyWriter(noHistoryTopic, "AAPL");
        writer.waitForReaders();
        // Wait until the reader has installed its onSamples callback: a sampleCount=0 reader keeps no history, so it
        // observes samples only through that callback, which must be registered before we publish.
        [[maybe_unused]] auto _ = makeSingleKeyReader(noHistoryBarrier, "barrier").getNextUnread();
        writer.add(make_shared<Stock>(12.0f, 13.0f, 14.0f));
        writer.partialUpdate<float>("price")(15.0f);
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // A reader that joins after a no-history (sampleCount == 0) writer's initial value is bootstrapped with the
    // writer's current per-key base value, so a subsequent partial update resolves against it rather than crashing on
    // a null base.
    Topic<string, StockPtr> lateJoinTopic(node, "lateJoinTopic");
    WriterConfig noHistoryWriter;
    noHistoryWriter.sampleCount = 0;
    lateJoinTopic.setWriterDefaultConfig(noHistoryWriter);
    lateJoinTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> lateJoinBarrier(node, "lateJoinBarrier");
    cout << "testing partial update to a late joiner of a no-history writer... " << flush;
    {
        auto writer = makeSingleKeyWriter(lateJoinTopic, "AAPL");

        // Publish the initial full value before any reader connects. The writer keeps no history, so this snapshot is
        // not retained and a later reader cannot be bootstrapped with it.
        writer.add(make_shared<Stock>(12.0f, 13.0f, 14.0f));

        // Let the reader attach as a late joiner, after the add.
        auto barrier = makeSingleKeyWriter(lateJoinBarrier, "barrier");
        barrier.waitForReaders();
        barrier.update(0);

        writer.waitForReaders();
        writer.partialUpdate<float>("price")(15.0f);            // resolves against the bootstrapped base
        writer.update(make_shared<Stock>(20.0f, 21.0f, 22.0f)); // a later full value
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // The base is also unavailable when the writer keeps history (sampleCount != 0) but the full sample has aged out
    // of that history (sampleLifetime). A late joiner then gets no init sample, yet must still be bootstrapped from
    // the per-key base, which is not subject to the sample lifetime.
    Topic<string, StockPtr> lifetimeTopic(node, "lifetimeTopic");
    WriterConfig lifetimeWriter;
    lifetimeWriter.sampleCount = -1;     // keep history...
    lifetimeWriter.sampleLifetime = 100; // ...but only for 100ms
    lifetimeWriter.clearHistory = ClearHistoryPolicy::Never;
    lifetimeTopic.setWriterDefaultConfig(lifetimeWriter);
    lifetimeTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> lifetimeBarrier(node, "lifetimeBarrier");
    cout << "testing partial update to a late joiner after the base aged out (sampleLifetime)... " << flush;
    {
        auto writer = makeSingleKeyWriter(lifetimeTopic, "AAPL");
        writer.add(make_shared<Stock>(12.0f, 13.0f, 14.0f));

        // Wait until the full sample has aged out of the writer's history, then let the reader join.
        this_thread::sleep_for(chrono::milliseconds(500));

        auto barrier = makeSingleKeyWriter(lifetimeBarrier, "barrier");
        barrier.waitForReaders();
        barrier.update(0);

        writer.waitForReaders();
        writer.partialUpdate<float>("price")(15.0f);
        writer.update(make_shared<Stock>(20.0f, 21.0f, 22.0f));
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // A reader that keeps no history (sampleCount == 0) and joins after the writer's add is bootstrapped with the base
    // value (sent even though it keeps no history), so a subsequent partial update resolves against it.
    Topic<string, StockPtr> lateReaderTopic(node, "lateReaderTopic");
    lateReaderTopic.setWriterDefaultConfig(config); // writer keeps history
    lateReaderTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> lateReaderJoinBarrier(node, "lateReaderJoinBarrier");
    Topic<string, int> lateReaderReadyBarrier(node, "lateReaderReadyBarrier");
    cout << "testing partial update to a late-joining sampleCount=0 reader... " << flush;
    {
        auto writer = makeSingleKeyWriter(lateReaderTopic, "AAPL");
        writer.add(make_shared<Stock>(12.0f, 13.0f, 14.0f));

        // Tell the reader the add is published; it can now join as a late joiner.
        auto joinBarrier = makeSingleKeyWriter(lateReaderJoinBarrier, "barrier");
        joinBarrier.waitForReaders();
        joinBarrier.update(0);

        // Wait until the reader has installed its onSamples callback before publishing the partial. The barrier
        // only orders the callback installation; also wait for the reader's element to attach, otherwise
        // waitForNoReaders below can return before the reader ever attached.
        [[maybe_unused]] auto _ = makeSingleKeyReader(lateReaderReadyBarrier, "barrier").getNextUnread();
        writer.waitForReaders();

        writer.partialUpdate<float>("price")(15.0f);
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // An any-key (or filtered) reader fetches all keys in a single request. With the default history policy
    // (ClearHistory=OnAll) the writer keeps only the newest sample, so a late joiner receives the newest key's
    // value and every other key's base must be bootstrapped from the per-key base, not merged onto a null base.
    Topic<string, StockPtr> trimTopic(node, "trimHistoryTopic"); // default config: ClearHistory=OnAll
    trimTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> trimBarrier(node, "trimHistoryBarrier");
    cout << "testing partial update to a late any-key reader after the writer trimmed the key's history... " << flush;
    {
        auto writer = makeAnyKeyWriter(trimTopic);
        writer.add("AAPL", make_shared<Stock>(12.0f, 13.0f, 14.0f));
        writer.add("GOOG", make_shared<Stock>(100.0f, 101.0f, 102.0f)); // clears history: only GOOG's full value kept

        auto barrier = makeSingleKeyWriter(trimBarrier, "barrier");
        barrier.waitForReaders();
        barrier.update(0);

        writer.waitForReaders();
        writer.partialUpdate<float>("price")("AAPL", 15.0f); // AAPL's full value was trimmed; must still resolve
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // When a reader with a limited sampleCount joins a writer whose number of distinct keys exceeds that sampleCount,
    // capping the init batch must keep one value per key (a resolvable base), not just the newest N samples overall.
    // Here three keys (MSFT, AAPL, GOOG) share a sampleCount=2 reader and GOOG has extra history, so a naive "newest
    // two samples" cap would drop MSFT's base entirely. The cap must keep it so a later partial update on MSFT
    // resolves.
    Topic<string, StockPtr> capTopic(node, "capTopic");
    capTopic.setWriterDefaultConfig(config); // keep full history
    capTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> capBarrier(node, "capBarrier");
    cout << "testing sampleCount cap keeps a base per key... " << flush;
    {
        auto writer = makeAnyKeyWriter(capTopic);
        writer.add("MSFT", make_shared<Stock>(50.0f, 51.0f, 52.0f));       // id 1 (oldest)
        writer.add("AAPL", make_shared<Stock>(12.0f, 13.0f, 14.0f));       // id 2
        writer.add("GOOG", make_shared<Stock>(100.0f, 101.0f, 102.0f));    // id 3
        writer.update("GOOG", make_shared<Stock>(200.0f, 201.0f, 202.0f)); // id 4, two GOOG samples

        auto barrier = makeSingleKeyWriter(capBarrier, "barrier");
        barrier.waitForReaders();
        barrier.update(0);

        writer.waitForReaders();
        writer.partialUpdate<float>("price")("MSFT", 55.0f); // resolves only if MSFT's base survived the cap
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // The reader-side updater throws on one of the initialization samples: only that sample is dropped, the other
    // initialization samples are delivered, and the following partial update resolves against the last successfully
    // applied value.
    Topic<string, StockPtr> throwTopic(node, "throwingUpdater");
    throwTopic.setWriterDefaultConfig(config); // keep full history
    throwTopic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });
    Topic<string, int> throwBarrier(node, "throwingUpdaterBarrier");
    cout << "testing reader-side updater exception... " << flush;
    {
        auto writer = makeSingleKeyWriter(throwTopic, "AAPL");
        writer.add(make_shared<Stock>(12.0f, 13.0f, 14.0f));
        writer.partialUpdate<float>("price")(-99.0f); // the reader's updater throws on negative prices
        writer.partialUpdate<float>("price")(15.0f);

        // Let the reader join late, so the three samples above are delivered as initialization samples.
        auto barrier = makeSingleKeyWriter(throwBarrier, "barrier");
        barrier.waitForReaders();
        barrier.update(0);

        writer.waitForReaders();
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
