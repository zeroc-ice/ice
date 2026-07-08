// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "Test.h"
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
    config.sampleCount = -1; // Unlimited sample count
    config.clearHistory = ClearHistoryPolicy::Never;

    cout << "testing single key reader/writer... " << flush;
    {
        {
            Topic<string, string> topic(node, "string");
            auto writer = makeSingleKeyWriter(topic, "elem1", "", config);

            writer.waitForReaders(1);
            test(writer.hasReaders());

            writer.add("value1");
            writer.update("value2");
            writer.remove();

            writer.waitForNoReaders();

            auto writer2 = makeSingleKeyWriter(topic, "elem2", "", config);
            writer2.waitForReaders(2);
            writer2.add("value");
            writer2.waitForNoReaders();
        }
        {
            Topic<int, Test::StructValue> topic(node, "struct");
            auto writer = makeSingleKeyWriter(topic, 10, "", config);

            writer.waitForReaders(1);
            test(writer.hasReaders());

            writer.add({"firstName", "lastName", 10});
            writer.update({"firstName", "lastName", 11});
            writer.remove();

            writer.waitForNoReaders();
        }
        {
            Topic<string, shared_ptr<Test::Base>> topic(node, "baseclass");
            auto writer = makeSingleKeyWriter(topic, "elem1", "", config);

            writer.waitForReaders(1);
            test(writer.hasReaders());

            writer.add(make_shared<Test::Base>("value1"));
            writer.update(make_shared<Test::Base>("value2"));
            writer.remove();

            writer.waitForNoReaders();
        }
        {
            Topic<string, shared_ptr<Test::Base>> topic(node, "baseclass2");
            {
                auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
                writer.waitForReaders(1);
                test(writer.hasReaders());

                writer.add(make_shared<Test::Base>("value1"));
                writer.update(make_shared<Test::Base>("value2"));
                writer.remove();
                writer.waitForNoReaders();
            }
            {
                auto writer = makeSingleKeyWriter(topic, "elem2", "", config);
                writer.waitForReaders(1);
                writer.update(make_shared<Test::Base>("value1"));
                writer.waitForNoReaders();
            }
            {
                auto writer = makeSingleKeyWriter(topic, "elem3", "", config);
                writer.waitForReaders(1);
                writer.remove();
                writer.waitForNoReaders();
            }
            {
                auto writer = makeSingleKeyWriter(topic, "elem4", "", config);
                writer.waitForReaders(1);
                writer.add(make_shared<Test::Base>("value1"));
                writer.waitForNoReaders();
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing multi-key reader/writer... " << flush;
    {
        {
            Topic<string, string> topic(node, "multikey1");
            auto writer1 = makeSingleKeyWriter(topic, "elem1", "", config);
            auto writer2 = makeSingleKeyWriter(topic, "elem2", "", config);

            writer1.waitForReaders(1);
            writer2.waitForReaders(1);

            writer1.add("value1");
            writer1.update("value2");
            writer1.remove();

            writer2.add("value1");
            writer2.update("value2");
            writer2.remove();

            writer1.waitForNoReaders();
            writer2.waitForNoReaders();
        }
    }
    cout << "ok" << endl;

    cout << "testing any-key reader/writer... " << flush;
    {
        {
            Topic<string, string> topic(node, "anykey1");
            auto writer1 = makeSingleKeyWriter(topic, "elem1", "", config);
            auto writer2 = makeSingleKeyWriter(topic, "elem2", "", config);

            writer1.waitForReaders(1);
            writer2.waitForReaders(1);

            writer1.add("value1");
            writer1.update("value2");
            writer1.remove();

            writer2.add("value1");
            writer2.update("value2");
            writer2.remove();

            writer1.waitForNoReaders();
            writer2.waitForNoReaders();
        }
    }
    cout << "ok" << endl;

    cout << "testing reader/multi-key writer... " << flush;
    {
        {
            Topic<string, string> topic(node, "multikey2");
            auto writer = makeMultiKeyWriter(topic, {"elem1", "elem2"}, "", config);

            writer.waitForReaders(2);

            writer.add("elem1", "value1");
            writer.update("elem1", "value2");
            writer.remove("elem1");

            writer.add("elem2", "value1");
            writer.update("elem2", "value2");
            writer.remove("elem2");

            writer.waitForNoReaders();
        }
    }
    cout << "ok" << endl;

    cout << "testing reader/any-key writer... " << flush;
    {
        {
            Topic<string, string> topic(node, "anykey2");
            auto writer = makeAnyKeyWriter(topic, "", config);

            writer.waitForReaders(2);

            writer.add("elem1", "value1");
            writer.update("elem1", "value2");
            writer.remove("elem1");

            writer.add("elem2", "value1");
            writer.update("elem2", "value2");
            writer.remove("elem2");

            writer.waitForNoReaders();
        }
    }
    cout << "ok" << endl;

    cout << "testing multi-key reader/multi-key writer... " << flush;
    {
        {
            Topic<string, string> topic(node, "multikey3");
            auto writer = makeMultiKeyWriter(topic, {"elem1", "elem2"}, "", config);

            writer.waitForReaders(1);

            writer.add("elem1", "value1");
            writer.update("elem1", "value2");
            writer.remove("elem1");

            writer.add("elem2", "value1");
            writer.update("elem2", "value2");
            writer.remove("elem2");

            writer.waitForNoReaders();
        }
    }
    cout << "ok" << endl;

    cout << "testing any-key reader/any-key writer... " << flush;
    {
        {
            Topic<string, string> topic(node, "anykey3");
            auto writer = makeAnyKeyWriter(topic, "", config);

            writer.waitForReaders(1);

            writer.add("elem1", "value1");
            writer.update("elem1", "value2");
            writer.remove("elem1");

            writer.add("elem2", "value1");
            writer.update("elem2", "value2");
            writer.remove("elem2");

            writer.waitForNoReaders();
        }
    }
    cout << "ok" << endl;

    cout << "testing filtered reader/writer... " << flush;
    {
        Topic<string, shared_ptr<Test::Base>> topic(node, "filtered1");

        topic.setKeyFilter<string>(
            "startswith",
            [](const string& prefix)
            {
                return [prefix](const string& key)
                { return key.size() >= prefix.size() && key.compare(0, prefix.size(), prefix) == 0; };
            });

        {
            auto writer5 = makeSingleKeyWriter(topic, "elem5", "", config);
            writer5.add(make_shared<Test::Base>("value1"));
            writer5.update(make_shared<Test::Base>("value2"));
            writer5.remove();

            auto writer1 = makeSingleKeyWriter(topic, "elem1", "", config);
            writer1.waitForReaders(1);
            test(writer1.hasReaders());
            writer1.add(make_shared<Test::Base>("value1"));
            writer1.update(make_shared<Test::Base>("value2"));
            writer1.remove();

            auto writer2 = makeSingleKeyWriter(topic, "elem2", "", config);
            writer2.waitForReaders(1);
            writer2.update(make_shared<Test::Base>("value1"));

            auto writer3 = makeSingleKeyWriter(topic, "elem3", "", config);
            writer3.waitForReaders(1);
            writer3.remove();

            auto writer4 = makeSingleKeyWriter(topic, "elem4", "", config);
            writer4.waitForReaders(1);
            writer4.add(make_shared<Test::Base>("value1"));
            writer4.waitForNoReaders();
        }
        {
            auto writer1 = makeSingleKeyWriter(topic, "nonvalue", "", config);
            auto writer2 = makeSingleKeyWriter(topic, "value", "", config);
            test(!writer1.hasReaders());
            writer2.waitForReaders(1);
            test(!writer1.hasReaders());
            writer2.remove();
            writer2.waitForNoReaders();
        }
    }
    cout << "ok" << endl;

    cout << "testing filtered reader/multi-key writer... " << flush;
    {
        Topic<string, shared_ptr<Test::Base>> topic(node, "filtered2");

        auto writer = makeMultiKeyWriter(topic, {"elem1", "elem2", "elem3", "elem4", "elem5"}, "", config);
        writer.waitForReaders(1);

        writer.add("elem5", make_shared<Test::Base>("value1"));
        writer.update("elem5", make_shared<Test::Base>("value2"));
        writer.remove("elem5");

        test(writer.hasReaders());
        writer.add("elem1", make_shared<Test::Base>("value1"));
        writer.update("elem1", make_shared<Test::Base>("value2"));
        writer.remove("elem1");

        writer.update("elem2", make_shared<Test::Base>("value1"));

        writer.remove("elem3");

        writer.add("elem4", make_shared<Test::Base>("value1"));
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    cout << "testing filtered reader/any-key writer... " << flush;
    {
        Topic<string, shared_ptr<Test::Base>> topic(node, "filtered3");

        auto writer = makeAnyKeyWriter(topic, "", config);
        writer.waitForReaders(1);

        writer.add("elem5", make_shared<Test::Base>("value1"));
        writer.update("elem5", make_shared<Test::Base>("value2"));
        writer.remove("elem5");

        test(writer.hasReaders());
        writer.add("elem1", make_shared<Test::Base>("value1"));
        writer.update("elem1", make_shared<Test::Base>("value2"));
        writer.remove("elem1");

        writer.update("elem2", make_shared<Test::Base>("value1"));

        writer.remove("elem3");

        writer.add("elem4", make_shared<Test::Base>("value1"));
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    cout << "testing filtered sample reader... " << flush;
    {
        Topic<string, string> topic(node, "filtered reader key/value filter");
        topic.setSampleFilter<string>(
            "startswith",
            [](const string& prefix)
            {
                return [prefix](const Sample<string, string>& sample)
                {
                    const auto& value = sample.getValue();
                    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
                };
            });

        auto writer1 = makeSingleKeyWriter(topic, "elem1", "", config);
        writer1.waitForReaders(3);
        test(writer1.hasReaders());
        writer1.add("value1");
        writer1.update("value2");
        writer1.remove();

        auto writer2 = makeSingleKeyWriter(topic, "elem2", "", config);
        writer2.waitForReaders(1);
        writer2.update("value1");
        writer2.update("value2");
        writer2.update("value3");
        writer2.update("value4");
        writer2.update("value5");

        auto writer3 = makeSingleKeyWriter(topic, "elem3", "", config);
        writer3.waitForReaders(1);
        writer3.update("nonvalue");
        writer3.update("value");

        writer1.waitForNoReaders();
        writer2.waitForNoReaders();
    }
    cout << "ok" << endl;

    // An any-key reader and a filtered reader coexisting on the same topic must each keep their own subscription:
    // both receive matching samples, and destroying one must not detach the other.
    cout << "testing coexisting any-key and filtered readers... " << flush;
    {
        Topic<string, string> topic(node, "readerCoexistence");
        Topic<string, int> barrier(node, "readerCoexistenceBarrier");

        auto writer = makeAnyKeyWriter(topic, "", config);
        writer.waitForReaders(2);  // the any-key reader and the filtered reader
        writer.update("k0", "v0"); // matches the filter, delivered to both readers

        // Wait until the peer destroyed the filtered reader.
        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "barrier").getNextUnread();

        writer.update("k1", "v1"); // the any-key reader must still be subscribed and receive this
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // A multi-key writer must only forward a key's samples to the sessions subscribed to that key: a session whose
    // only reader subscribes elem1 must not receive elem2's samples; an any-key reader receives every key.
    cout << "testing multi-key writer with a partially subscribed session... " << flush;
    {
        Topic<string, string> topic(node, "unmatchedKey");

        auto writer = makeMultiKeyWriter(topic, {"elem1", "elem2"}, "", config);
        writer.waitForReaders(2);         // the single-key reader and the any-key reader
        writer.update("elem2", "value2"); // only the any-key reader subscribes elem2
        writer.update("elem1", "value1");
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // A late-joining reader of a multi-key writer must receive the initialization samples of every key it
    // subscribes to, not just the first key's batch.
    cout << "testing late-joining multi-key reader... " << flush;
    {
        Topic<string, string> topic(node, "lateJoinMultiKey");
        Topic<string, int> barrier(node, "lateJoinMultiKeyBarrier");
        Topic<string, int> done(node, "lateJoinMultiKeyDone");

        auto writer = makeMultiKeyWriter(topic, {"elemA", "elemB"}, "", config);
        writer.add("elemA", "valueA");
        writer.add("elemB", "valueB");

        // The reader side attaches (and destroys) a probe first, then waits on the barrier before creating the
        // late-joining reader.
        auto barrierWriter = makeSingleKeyWriter(barrier, "barrier");
        barrierWriter.waitForReaders();
        barrierWriter.update(0);

        // Gate teardown on the reader signalling that both its readers were initialized, so this writer never races
        // the late readers' lifetime through a listener count that can drop back to zero before it is observed.
        [[maybe_unused]] auto _ = makeSingleKeyReader(done, "done").getNextUnread();
    }
    cout << "ok" << endl;

    // Two late-joining single-key readers of a multi-key writer must each receive their own key's initialization
    // samples: the batches target the same writer element and must be routed per reader.
    cout << "testing late-joining single-key readers... " << flush;
    {
        Topic<string, string> topic(node, "lateJoinSingleKeys");
        Topic<string, int> barrier(node, "lateJoinSingleKeysBarrier");
        Topic<string, int> done(node, "lateJoinSingleKeysDone");

        auto writer = makeMultiKeyWriter(topic, {"elemA", "elemB"}, "", config);
        writer.add("elemA", "valueA");
        writer.add("elemB", "valueB");

        // See the previous case regarding the reader-side probe.
        auto barrierWriter = makeSingleKeyWriter(barrier, "barrier");
        barrierWriter.waitForReaders();
        barrierWriter.update(0);

        // Gate teardown on the reader signalling that both readers were initialized, so this writer never races their
        // lifetime through a listener count that can drop back to zero before it is observed.
        [[maybe_unused]] auto _ = makeSingleKeyReader(done, "done").getNextUnread();
    }
    cout << "ok" << endl;

    // Two readers on the same key, created back-to-back, attach to the writer element in one initialization round.
    // Each must receive the key's initialization sample exactly once; the live update below, published only after both
    // readers drained their initialization sample, would surface a duplicated batch as a second "valueA" ahead of it.
    cout << "testing coalesced same-key readers... " << flush;
    {
        Topic<string, string> topic(node, "coalescedSameKey");
        Topic<string, int> barrier(node, "coalescedSameKeyBarrier");
        Topic<string, int> ready(node, "coalescedSameKeyReady");

        auto writer = makeSingleKeyWriter(topic, "elemA", "", config);
        writer.add("valueA");

        auto barrierWriter = makeSingleKeyWriter(barrier, "barrier");
        barrierWriter.waitForReaders();
        barrierWriter.update(0);

        // Gate the live update on the readers signalling that they attached and drained one initialization sample, so
        // this writer never races the late readers' lifetime through waitForReaders/waitForNoReaders.
        [[maybe_unused]] auto _ = makeSingleKeyReader(ready, "ready").getNextUnread();
        writer.update("valueB");
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // A late-joining filtered reader must receive the initialization samples of exactly the writer keys its filter
    // matches, and must stay subscribed: the addressed return acknowledgment must route by key without detaching it.
    cout << "testing late-joining filtered reader... " << flush;
    {
        Topic<string, string> topic(node, "lateFilter");
        Topic<string, int> barrier(node, "lateFilterBarrier");
        Topic<string, int> ready(node, "lateFilterReady");

        auto writer = makeMultiKeyWriter(topic, {"elem1", "elem2", "other"}, "", config);
        writer.add("elem1", "value1");
        writer.add("elem2", "value2");
        writer.add("other", "valueOther");

        auto barrierWriter = makeSingleKeyWriter(barrier, "barrier");
        barrierWriter.waitForReaders();
        barrierWriter.update(0);

        // Publish a live update on a matching key only after the reader drained its initialization samples. The reader
        // must still be subscribed to receive it: a mis-addressed return acknowledgment would have detached it.
        [[maybe_unused]] auto _ = makeSingleKeyReader(ready, "ready").getNextUnread();
        writer.update("elem1", "value1Live");
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // A late-joining reader of a key the writer covers but never wrote receives an empty initialization batch. The
    // reader must still be marked initialized so a later live sample on that key is delivered.
    cout << "testing late-joining reader of an unwritten key... " << flush;
    {
        Topic<string, string> topic(node, "lateEmptyBatch");
        Topic<string, int> barrier(node, "lateEmptyBatchBarrier");
        Topic<string, int> ready(node, "lateEmptyBatchReady");

        auto writer = makeMultiKeyWriter(topic, {"elemA", "elemB"}, "", config);
        writer.add("elemA", "valueA"); // elemB is covered but never written

        auto barrierWriter = makeSingleKeyWriter(barrier, "barrier");
        barrierWriter.waitForReaders();
        barrierWriter.update(0);

        // Delivered only if the empty initialization batch for elemB marked the reader initialized.
        [[maybe_unused]] auto _ = makeSingleKeyReader(ready, "ready").getNextUnread();
        writer.update("elemB", "valueB");
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;

    // A late-joining multi-key reader must receive every key's initialization samples even when the sample ids
    // interleave across keys (elemA: 1, 3; elemB: 2); the receiver must not drop the lower-id batch as already seen.
    cout << "testing late-joining reader with interleaved sample ids... " << flush;
    {
        Topic<string, string> topic(node, "lateInterleaved");
        Topic<string, int> barrier(node, "lateInterleavedBarrier");
        Topic<string, int> done(node, "lateInterleavedDone");

        auto writer = makeMultiKeyWriter(topic, {"elemA", "elemB"}, "", config);
        writer.add("elemA", "valueA1");    // id 1
        writer.add("elemB", "valueB1");    // id 2
        writer.update("elemA", "valueA2"); // id 3

        auto barrierWriter = makeSingleKeyWriter(barrier, "barrier");
        barrierWriter.waitForReaders();
        barrierWriter.update(0);

        // Gate teardown on the reader signalling completion, so this writer does not race the late reader's lifetime.
        [[maybe_unused]] auto _ = makeSingleKeyReader(done, "done").getNextUnread();
    }
    cout << "ok" << endl;

    // A late-joining reader of an any-key writer must receive every key's initialization samples. An any-key writer
    // is registered under the always-match filter, so its addressed return acknowledgments travel the filter branch.
    cout << "testing late-joining reader of an any-key writer... " << flush;
    {
        Topic<string, string> topic(node, "lateAnyKeyWriter");
        Topic<string, int> barrier(node, "lateAnyKeyWriterBarrier");
        Topic<string, int> done(node, "lateAnyKeyWriterDone");

        auto writer = makeAnyKeyWriter(topic, "", config);
        writer.add("elemA", "valueA");
        writer.add("elemB", "valueB");

        auto barrierWriter = makeSingleKeyWriter(barrier, "barrier");
        barrierWriter.waitForReaders();
        barrierWriter.update(0);

        [[maybe_unused]] auto _ = makeSingleKeyReader(done, "done").getNextUnread();
    }
    cout << "ok" << endl;

    cout << "testing topic collocated key reader and writer... " << flush;
    {
        Topic<string, string> topic(node, "collocated");
        {
            auto writer = makeSingleKeyWriter(topic, "test");
            writer.add("add");

            auto reader = makeSingleKeyReader(topic, "test");
            test(reader.getNextUnread().getValue() == "add");
        }
        {
            auto reader = makeSingleKeyReader(topic, "test");

            auto writer = makeSingleKeyWriter(topic, "test");
            writer.update("update");

            test(reader.getNextUnread().getValue() == "update");
        }
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
