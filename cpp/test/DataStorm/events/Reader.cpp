// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "Test.h"
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
    config.sampleCount = -1; // Unlimited sample count
    config.clearHistory = ClearHistoryPolicy::Never;

    {
        Topic<string, string> topic(node, "string");
        {
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);

            reader.waitForWriters(1);
            test(reader.hasWriters());

            auto testSample = [&reader](SampleEvent event, const string& key, const string& value = "")
            {
                reader.waitForUnread(1);
                auto sample = reader.getNextUnread();
                test(sample.getKey() == key);
                test(sample.getEvent() == event);
                if (event != SampleEvent::Remove)
                {
                    test(sample.getValue() == value);
                }
            };

            testSample(SampleEvent::Add, "elem1", "value1");
            testSample(SampleEvent::Update, "elem1", "value2");
            testSample(SampleEvent::Remove, "elem1");

            test(reader.getAllUnread().empty());
        }
        {
            auto reader1 = makeSingleKeyReader(topic, "elem2", "", config);
            auto reader2 = makeSingleKeyReader(topic, "elem2", "", config);
            reader1.waitForWriters(1);
            reader2.waitForWriters(1);
            reader1.waitForUnread();
            reader2.waitForUnread();
        }
    }

    {
        Topic<int, Test::StructValue> topic(node, "struct");
        auto reader = makeSingleKeyReader(topic, 10, "", config);

        reader.waitForWriters(1);
        test(reader.hasWriters());

        auto testSample = [&reader](SampleEvent event, const Test::StructValue& value = Test::StructValue())
        {
            reader.waitForUnread(1);
            auto sample = reader.getNextUnread();
            test(sample.getKey() == 10);
            test(sample.getEvent() == event);
            if (event != SampleEvent::Remove)
            {
                test(sample.getValue() == value);
            }
        };

        testSample(SampleEvent::Add, Test::StructValue({"firstName", "lastName", 10}));
        testSample(SampleEvent::Update, Test::StructValue({"firstName", "lastName", 11}));
        testSample(SampleEvent::Remove);
    }

    {
        Topic<string, shared_ptr<Test::Base>> topic(node, "baseclass");
        auto reader = makeSingleKeyReader(topic, "elem1", "", config);

        reader.waitForWriters(1);
        test(reader.hasWriters());

        auto testSample = [&reader](SampleEvent event, const string& value = "")
        {
            reader.waitForUnread(1);
            auto sample = reader.getNextUnread();
            test(sample.getKey() == "elem1");
            test(sample.getEvent() == event);
            if (event != SampleEvent::Remove)
            {
                test(sample.getValue()->b == value);
            }
        };

        testSample(SampleEvent::Add, "value1");
        testSample(SampleEvent::Update, "value2");
        testSample(SampleEvent::Remove);
    }

    {
        Topic<string, shared_ptr<Test::Base>> topic(node, "baseclass2");

        auto testSample = [](typename decltype(topic)::ReaderType& reader,
                             SampleEvent event,
                             const string& key,
                             const string& value = "")
        {
            reader.waitForWriters(1);
            test(reader.hasWriters());

            reader.waitForUnread(1);
            auto sample = reader.getNextUnread();
            test(sample.getKey() == key);
            test(sample.getEvent() == event);
            if (event != SampleEvent::Remove)
            {
                test(sample.getValue()->b == value);
            }
        };

        {
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            testSample(reader, SampleEvent::Add, "elem1", "value1");
            testSample(reader, SampleEvent::Update, "elem1", "value2");
            testSample(reader, SampleEvent::Remove, "elem1");
        }
        {
            auto reader = makeSingleKeyReader(topic, "elem2", "", config);
            testSample(reader, SampleEvent::Update, "elem2", "value1");
        }
        {
            auto reader = makeSingleKeyReader(topic, "elem3", "", config);
            testSample(reader, SampleEvent::Remove, "elem3");
        }
        {
            auto reader = makeSingleKeyReader(topic, "elem4", "", config);
            testSample(reader, SampleEvent::Add, "elem4", "value1");
        }
    }

    {
        Topic<string, string> topic(node, "multikey1");

        auto reader = makeMultiKeyReader(topic, {"elem1", "elem2"}, "", config);
        reader.waitForWriters(2);
        reader.waitForUnread(6);
        test(reader.getAllUnread().size() == 6);
    }

    {
        Topic<string, string> topic(node, "anykey1");

        auto reader = makeAnyKeyReader(topic, "", config);
        reader.waitForWriters(2);
        reader.waitForUnread(6);
        test(reader.getAllUnread().size() == 6);
    }

    {
        Topic<string, string> topic(node, "multikey2");

        auto reader1 = makeSingleKeyReader(topic, "elem1", "", config);
        auto reader2 = makeSingleKeyReader(topic, "elem2", "", config);

        reader1.waitForWriters(1);
        reader1.waitForUnread(3);
        test(reader1.getAllUnread().size() == 3);

        reader2.waitForWriters(1);
        reader2.waitForUnread(3);
        test(reader2.getAllUnread().size() == 3);
    }

    {
        Topic<string, string> topic(node, "anykey2");

        auto reader1 = makeSingleKeyReader(topic, "elem1", "", config);
        auto reader2 = makeSingleKeyReader(topic, "elem2", "", config);

        reader1.waitForWriters(1);
        reader1.waitForUnread(3);
        test(reader1.getAllUnread().size() == 3);

        reader2.waitForWriters(1);
        reader2.waitForUnread(3);
        test(reader2.getAllUnread().size() == 3);
    }

    {
        Topic<string, string> topic(node, "multikey3");

        auto reader = makeMultiKeyReader(topic, {"elem1", "elem2"}, "", config);
        reader.waitForWriters(2);
        reader.waitForUnread(6);
        test(reader.getAllUnread().size() == 6);
    }

    {
        Topic<string, string> topic(node, "anykey3");

        auto reader = makeAnyKeyReader(topic, "", config);
        reader.waitForWriters(1);
        reader.waitForUnread(6);
        test(reader.getAllUnread().size() == 6);
    }

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
            auto reader = makeFilteredKeyReader(topic, Filter<string>("_regex", "elem[0-4]"), "", config);

            reader.waitForWriters(1);
            test(reader.hasWriters());

            auto testSample = [&reader](SampleEvent event, const string& key, const string& value = "")
            {
                reader.waitForUnread(1);
                auto sample = reader.getNextUnread();
                test(sample.getKey() == key);
                test(sample.getEvent() == event);
                if (event != SampleEvent::Remove)
                {
                    test(sample.getValue()->b == value);
                }
            };

            testSample(SampleEvent::Add, "elem1", "value1");
            testSample(SampleEvent::Update, "elem1", "value2");
            testSample(SampleEvent::Remove, "elem1");

            testSample(SampleEvent::Update, "elem2", "value1");
            testSample(SampleEvent::Remove, "elem3");
            testSample(SampleEvent::Add, "elem4", "value1");
        }
        {
            auto reader = makeFilteredKeyReader(topic, Filter<string>("startswith", "val"), "", config);
            reader.waitForWriters(1);
            test(reader.hasWriters());
            auto sample = reader.getNextUnread();
            test(sample.getKey().find("val") == 0);
        }
    }

    {
        Topic<string, shared_ptr<Test::Base>> topic(node, "filtered2");

        auto reader = makeFilteredKeyReader(topic, Filter<string>("_regex", "elem[0-4]"), "", config);

        reader.waitForWriters(1);
        test(reader.hasWriters());

        auto testSample = [&reader](SampleEvent event, const string& key, const string& value = "")
        {
            reader.waitForUnread(1);
            auto sample = reader.getNextUnread();
            test(sample.getKey() == key);
            test(sample.getEvent() == event);
            if (event != SampleEvent::Remove)
            {
                test(sample.getValue()->b == value);
            }
        };

        testSample(SampleEvent::Add, "elem1", "value1");
        testSample(SampleEvent::Update, "elem1", "value2");
        testSample(SampleEvent::Remove, "elem1");

        testSample(SampleEvent::Update, "elem2", "value1");
        testSample(SampleEvent::Remove, "elem3");
        testSample(SampleEvent::Add, "elem4", "value1");
    }

    {
        Topic<string, shared_ptr<Test::Base>> topic(node, "filtered3");

        auto reader = makeFilteredKeyReader(topic, Filter<string>("_regex", "elem[0-4]"), "", config);

        reader.waitForWriters(1);
        test(reader.hasWriters());

        auto testSample = [&reader](SampleEvent event, const string& key, const string& value = "")
        {
            reader.waitForUnread(1);
            auto sample = reader.getNextUnread();
            test(sample.getKey() == key);
            test(sample.getEvent() == event);
            if (event != SampleEvent::Remove)
            {
                test(sample.getValue()->b == value);
            }
        };

        testSample(SampleEvent::Add, "elem1", "value1");
        testSample(SampleEvent::Update, "elem1", "value2");
        testSample(SampleEvent::Remove, "elem1");

        testSample(SampleEvent::Update, "elem2", "value1");
        testSample(SampleEvent::Remove, "elem3");
        testSample(SampleEvent::Add, "elem4", "value1");
    }

    {
        Topic<string, string> topic(node, "filtered reader key/value filter");

        {
            auto testSample = [](typename decltype(topic)::ReaderType& reader,
                                 SampleEvent event,
                                 const string& key,
                                 const string& value = "")
            {
                reader.waitForUnread(1);
                auto sample = reader.getNextUnread();
                test(sample.getKey() == key);
                test(sample.getEvent() == event);
                if (event != SampleEvent::Remove)
                {
                    test(sample.getValue() == value);
                }
            };

            auto reader11 = makeFilteredKeyReader(
                topic,
                Filter<string>("_regex", "elem[1]"),
                Filter<SampleEventSeq>("_event", SampleEventSeq{SampleEvent::Add}),
                "",
                config);
            auto reader12 = makeFilteredKeyReader(
                topic,
                Filter<string>("_regex", "elem[1]"),
                Filter<SampleEventSeq>("_event", SampleEventSeq{SampleEvent::Update}),
                "",
                config);
            auto reader13 = makeFilteredKeyReader(
                topic,
                Filter<string>("_regex", "elem[1]"),
                Filter<SampleEventSeq>("_event", SampleEventSeq{SampleEvent::Remove}),
                "",
                config);
            testSample(reader11, SampleEvent::Add, "elem1", "value1");
            testSample(reader12, SampleEvent::Update, "elem1", "value2");
            testSample(reader13, SampleEvent::Remove, "elem1");
        }
        {
            auto testSample = [](typename decltype(topic)::ReaderType& reader,
                                 SampleEvent event,
                                 const string& key,
                                 const string& value = "")
            {
                reader.waitForUnread(1);
                auto sample = reader.getNextUnread();
                test(sample.getKey() == key);
                test(sample.getEvent() == event);
                if (event != SampleEvent::Remove)
                {
                    test(sample.getValue() == value);
                }
            };

            auto reader2 = makeFilteredKeyReader(
                topic,
                Filter<string>("_regex", "elem[2]"),
                Filter<string>("_regex", "value[2-4]"),
                "",
                config);
            testSample(reader2, SampleEvent::Update, "elem2", "value2");
            testSample(reader2, SampleEvent::Update, "elem2", "value3");
            testSample(reader2, SampleEvent::Update, "elem2", "value4");
        }
        {
            auto testSample = [](typename decltype(topic)::ReaderType& reader,
                                 SampleEvent event,
                                 const string& key,
                                 const string& value = "")
            {
                reader.waitForUnread(1);
                auto sample = reader.getNextUnread();
                test(sample.getKey() == key);
                test(sample.getEvent() == event);
                if (event != SampleEvent::Remove)
                {
                    test(sample.getValue() == value);
                }
            };

            auto reader2 = makeSingleKeyReader(topic, "elem3", Filter<string>("startswith", "val"), "", config);
            testSample(reader2, SampleEvent::Update, "elem3", "value");
        }
    }

    // Coexisting any-key and filtered readers on the same topic: each keeps its own subscription. Both receive a
    // sample matching the filter, and destroying the filtered reader leaves the any-key reader subscribed.
    {
        Topic<string, string> topic(node, "readerCoexistence");
        Topic<string, int> barrier(node, "readerCoexistenceBarrier");

        auto anyKeyReader = makeAnyKeyReader(topic, "", config);
        {
            auto filteredReader = makeFilteredKeyReader(topic, Filter<string>("_regex", "k0"), "", config);
            filteredReader.waitForWriters(1);
            anyKeyReader.waitForWriters(1);

            auto sample = filteredReader.getNextUnread();
            test(sample.getKey() == "k0");
            test(sample.getValue() == "v0");
            sample = anyKeyReader.getNextUnread();
            test(sample.getKey() == "k0");
            test(sample.getValue() == "v0");
        }

        // Signal that the filtered reader is destroyed; the writer then publishes a second key.
        auto barrierWriter = makeSingleKeyWriter(barrier, "barrier");
        barrierWriter.waitForReaders();
        barrierWriter.update(0);

        auto sample = anyKeyReader.getNextUnread();
        test(sample.getKey() == "k1");
        test(sample.getValue() == "v1");
    }

    // This single-key reader must only receive its subscribed key from the multi-key writer, while the any-key
    // reader on the same session receives every key.
    {
        Topic<string, string> topic(node, "unmatchedKey");

        auto reader = makeSingleKeyReader(topic, "elem1", "", config);
        auto anyKeyReader = makeAnyKeyReader(topic, "", config);
        reader.waitForWriters(1);
        anyKeyReader.waitForWriters(1);

        auto sample = reader.getNextUnread();
        test(sample.getKey() == "elem1");
        test(sample.getValue() == "value1");
        test(!reader.hasUnread()); // elem2's sample was not delivered to the single-key reader

        // The any-key reader receives both keys, in publication order.
        sample = anyKeyReader.getNextUnread();
        test(sample.getKey() == "elem2");
        test(sample.getValue() == "value2");
        sample = anyKeyReader.getNextUnread();
        test(sample.getKey() == "elem1");
        test(sample.getValue() == "value1");
    }

    // A late-joining reader of a multi-key writer must receive the initialization samples of every key it
    // subscribes to.
    {
        Topic<string, string> topic(node, "lateJoinMultiKey");
        Topic<string, int> barrier(node, "lateJoinMultiKeyBarrier");
        Topic<string, int> done(node, "lateJoinMultiKeyDone");

        // Attach and destroy a probe reader first: once its element-level attach completed, the topics are
        // attached, so creating the reader below announces a new element on the already-attached session and the
        // writer initiates the element attach (the initialization samples then arrive via the initSamples request).
        {
            auto probe = makeSingleKeyReader(topic, "elemA", "", config);
            probe.waitForWriters(1);
        }

        // Wait until the writer published both keys.
        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "barrier").getNextUnread();

        auto reader = makeMultiKeyReader(topic, {"elemA", "elemB"}, "", config);
        reader.waitForUnread(2);
        map<string, string> values;
        for (const auto& sample : reader.getAllUnread())
        {
            test(sample.getEvent() == SampleEvent::Add);
            values[sample.getKey()] = sample.getValue();
        }
        test(values.size() == 2);
        test(values["elemA"] == "valueA");
        test(values["elemB"] == "valueB");

        // A late-joining any-key reader must also receive every key's initialization samples.
        auto anyKeyReader = makeAnyKeyReader(topic, "", config);
        anyKeyReader.waitForUnread(2);
        values.clear();
        for (const auto& sample : anyKeyReader.getAllUnread())
        {
            values[sample.getKey()] = sample.getValue();
        }
        test(values.size() == 2);
        test(values["elemA"] == "valueA");
        test(values["elemB"] == "valueB");

        // Signal the writer that both readers were initialized, so it can tear down.
        auto doneWriter = makeSingleKeyWriter(done, "done");
        doneWriter.waitForReaders();
        doneWriter.update(0);
    }

    // Two late-joining single-key readers of a multi-key writer must each receive their own key's initialization
    // samples, and neither the other reader's.
    {
        Topic<string, string> topic(node, "lateJoinSingleKeys");
        Topic<string, int> barrier(node, "lateJoinSingleKeysBarrier");
        Topic<string, int> done(node, "lateJoinSingleKeysDone");

        // Probe reader: see the previous case.
        {
            auto probe = makeSingleKeyReader(topic, "elemA", "", config);
            probe.waitForWriters(1);
        }

        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "barrier").getNextUnread();

        auto readerA = makeSingleKeyReader(topic, "elemA", "", config);
        auto readerB = makeSingleKeyReader(topic, "elemB", "", config);

        auto sample = readerA.getNextUnread();
        test(sample.getKey() == "elemA");
        test(sample.getValue() == "valueA");

        sample = readerB.getNextUnread();
        test(sample.getKey() == "elemB");
        test(sample.getValue() == "valueB");

        test(!readerA.hasUnread()); // readerA must not also receive elemB's sample
        test(!readerB.hasUnread()); // readerB must not also receive elemA's sample

        // Signal the writer that both readers were initialized, so it can tear down.
        auto doneWriter = makeSingleKeyWriter(done, "done");
        doneWriter.waitForReaders();
        doneWriter.update(0);
    }

    // Two readers on the same key, created back-to-back so they attach to the writer element in one initialization
    // round, must each receive the key's initialization sample exactly once. The live update "valueB" is published
    // only after both attach and drain, so a duplicated initialization sample would surface as a second "valueA"
    // ahead of it. This depends on the two readers coalescing into one round (very likely with back-to-back creation,
    // though not guaranteed); when they do not, the case still passes.
    {
        Topic<string, string> topic(node, "coalescedSameKey");
        Topic<string, int> barrier(node, "coalescedSameKeyBarrier");
        Topic<string, int> ready(node, "coalescedSameKeyReady");

        {
            auto probe = makeSingleKeyReader(topic, "elemA", "", config);
            probe.waitForWriters(1);
        }

        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "barrier").getNextUnread();

        auto readerA1 = makeSingleKeyReader(topic, "elemA", "", config);
        auto readerA2 = makeSingleKeyReader(topic, "elemA", "", config);

        auto initA1 = readerA1.getNextUnread();
        test(initA1.getKey() == "elemA" && initA1.getValue() == "valueA");
        auto initA2 = readerA2.getNextUnread();
        test(initA2.getKey() == "elemA" && initA2.getValue() == "valueA");

        auto readyWriter = makeSingleKeyWriter(ready, "ready");
        readyWriter.waitForReaders();
        readyWriter.update(0);

        // The next sample of each reader must be the live update, not a duplicated initialization sample.
        auto liveA1 = readerA1.getNextUnread();
        test(liveA1.getKey() == "elemA" && liveA1.getValue() == "valueB");
        test(!readerA1.hasUnread());

        auto liveA2 = readerA2.getNextUnread();
        test(liveA2.getKey() == "elemA" && liveA2.getValue() == "valueB");
        test(!readerA2.hasUnread());
    }

    // A late-joining filtered reader must receive the initialization samples of exactly the writer keys its filter
    // matches (not the writer's other keys), and must stay subscribed afterwards.
    {
        Topic<string, string> topic(node, "lateFilter");
        Topic<string, int> barrier(node, "lateFilterBarrier");
        Topic<string, int> ready(node, "lateFilterReady");

        {
            auto probe = makeSingleKeyReader(topic, "elem1", "", config);
            probe.waitForWriters(1);
        }

        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "barrier").getNextUnread();

        auto reader = makeFilteredKeyReader(topic, Filter<string>("_regex", "elem[0-9]"), "", config);
        reader.waitForUnread(2);
        map<string, string> values;
        for (const auto& sample : reader.getAllUnread())
        {
            values[sample.getKey()] = sample.getValue();
        }
        test(values.size() == 2); // "other" does not match the filter, so it is not delivered
        test(values["elem1"] == "value1");
        test(values["elem2"] == "value2");

        auto readyWriter = makeSingleKeyWriter(ready, "ready");
        readyWriter.waitForReaders();
        readyWriter.update(0);

        // The reader is still subscribed after initialization, so it receives the live update on a matching key.
        auto live = reader.getNextUnread();
        test(live.getKey() == "elem1");
        test(live.getValue() == "value1Live");
    }

    // A late-joining reader of a key the writer covers but never wrote receives an empty initialization batch; it must
    // still be marked initialized so a later live sample on that key is delivered.
    {
        Topic<string, string> topic(node, "lateEmptyBatch");
        Topic<string, int> barrier(node, "lateEmptyBatchBarrier");
        Topic<string, int> ready(node, "lateEmptyBatchReady");

        {
            auto probe = makeSingleKeyReader(topic, "elemA", "", config);
            probe.waitForWriters(1);
        }

        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "barrier").getNextUnread();

        auto reader = makeSingleKeyReader(topic, "elemB", "", config);
        reader.waitForWriters(1); // attached via the empty initialization batch; a live sample is now delivered

        auto readyWriter = makeSingleKeyWriter(ready, "ready");
        readyWriter.waitForReaders();
        readyWriter.update(0);

        // elemB had no queued sample, so initialization delivered nothing; the live update is the first sample.
        auto sample = reader.getNextUnread();
        test(sample.getKey() == "elemB");
        test(sample.getValue() == "valueB");
    }

    // A late-joining multi-key reader must receive every key's initialization samples even when the sample ids
    // interleave across keys (elemA: 1, 3; elemB: 2).
    {
        Topic<string, string> topic(node, "lateInterleaved");
        Topic<string, int> barrier(node, "lateInterleavedBarrier");
        Topic<string, int> done(node, "lateInterleavedDone");

        {
            auto probe = makeSingleKeyReader(topic, "elemA", "", config);
            probe.waitForWriters(1);
        }

        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "barrier").getNextUnread();

        auto reader = makeMultiKeyReader(topic, {"elemA", "elemB"}, "", config);
        reader.waitForUnread(3);
        map<string, vector<string>> byKey;
        for (const auto& sample : reader.getAllUnread())
        {
            byKey[sample.getKey()].push_back(sample.getValue());
        }
        test(byKey.size() == 2);
        test((byKey["elemA"] == vector<string>{"valueA1", "valueA2"}));
        test((byKey["elemB"] == vector<string>{"valueB1"}));

        auto doneWriter = makeSingleKeyWriter(done, "done");
        doneWriter.waitForReaders();
        doneWriter.update(0);
    }

    // A late-joining reader of an any-key writer must receive every key's initialization samples. This exercises the
    // any-key (always-match filter) writer branch.
    {
        Topic<string, string> topic(node, "lateAnyKeyWriter");
        Topic<string, int> barrier(node, "lateAnyKeyWriterBarrier");
        Topic<string, int> done(node, "lateAnyKeyWriterDone");

        {
            auto probe = makeSingleKeyReader(topic, "elemA", "", config);
            probe.waitForWriters(1);
        }

        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "barrier").getNextUnread();

        auto reader = makeMultiKeyReader(topic, {"elemA", "elemB"}, "", config);
        reader.waitForUnread(2);
        map<string, string> values;
        for (const auto& sample : reader.getAllUnread())
        {
            values[sample.getKey()] = sample.getValue();
        }
        test(values.size() == 2);
        test(values["elemA"] == "valueA");
        test(values["elemB"] == "valueB");

        auto doneWriter = makeSingleKeyWriter(done, "done");
        doneWriter.waitForReaders();
        doneWriter.update(0);
    }
}

DEFINE_TEST(::Reader)
