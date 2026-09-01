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
        // Two readers of one any-key writer, each with a key filter that throws on the key the other one expects.
        // Every sample reaches the reader whose filter accepts it, whichever order the writer element serves them in.
        Topic<string, string> topic(node, "keyFilterThrow");
        topic.setKeyFilter<string>(
            "throwOnKey",
            [](const string& boom)
            {
                return [boom](const string& key)
                {
                    if (key == boom)
                    {
                        throw runtime_error("the key filter failed");
                    }
                    return true;
                };
            });

        auto reader1 = makeFilteredKeyReader(topic, Filter<string>("throwOnKey", "k1"), "", config);
        auto reader2 = makeFilteredKeyReader(topic, Filter<string>("throwOnKey", "k2"), "", config);
        reader1.waitForWriters(1);
        reader2.waitForWriters(1);

        test(reader1.getNextUnread().getKey() == "k2");
        test(reader1.getNextUnread().getKey() == "sentinel");

        test(reader2.getNextUnread().getKey() == "k1");
        test(reader2.getNextUnread().getKey() == "sentinel");
    }

    {
        // One filtered reader over two keyed writers, with a key filter that throws on "k1". The reader attaches to
        // the writer of "k2" and receives its samples; the writer of "k1" stays unattached.
        Topic<string, string> topic(node, "attachKeyFilterThrow");
        topic.setKeyFilter<string>(
            "throwOnKey",
            [](const string& boom)
            {
                return [boom](const string& key)
                {
                    if (key == boom)
                    {
                        throw runtime_error("the key filter failed");
                    }
                    return true;
                };
            });

        auto reader = makeFilteredKeyReader(topic, Filter<string>("throwOnKey", "k1"), "", config);
        reader.waitForWriters(1);

        auto sample = reader.getNextUnread();
        test(sample.getKey() == "k2");
        test(sample.getValue() == "v2");
    }

    {
        // The writer queues k1, k2 and sentinel before this reader attaches, so they arrive as one initialization
        // batch. The filter throws on k1: that sample is dropped like a rejected key, and k2 and sentinel are still
        // delivered.
        Topic<string, string> topic(node, "initKeyFilterThrow");
        topic.setKeyFilter<string>(
            "throwOnKey",
            [](const string& boom)
            {
                return [boom](const string& key)
                {
                    if (key == boom)
                    {
                        throw runtime_error("the key filter failed");
                    }
                    return true;
                };
            });

        Topic<string, string> readyTopic(node, "initKeyFilterThrowReady");
        auto ready = makeSingleKeyReader(readyTopic, "ready", "", config);
        test(ready.getNextUnread().getValue() == "go");

        auto reader = makeFilteredKeyReader(topic, Filter<string>("throwOnKey", "k1"), "", config);
        test(reader.getNextUnread().getKey() == "k2");
        test(reader.getNextUnread().getKey() == "sentinel");

        auto done = makeSingleKeyWriter(readyTopic, "done");
        done.waitForReaders(1);
        done.add("done");
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

    // An unfiltered reader and a sample-filtered reader on the same key of the same writer. The writer forwards a
    // matching sample to the filtered reader's facet as well as to the unfaceted destination, and the unfiltered
    // reader must receive it once: the writer publishes "a" and then "b", so this reader reads "a" then "b".
    {
        Topic<string, string> topic(node, "unfilteredWithSampleFilter");

        auto plain = makeSingleKeyReader(topic, "elem", "", config);
        auto filtered = makeSingleKeyReader(topic, "elem", Filter<string>("contains", "a"), "", config);

        test(plain.getNextUnread().getValue() == "a");
        test(plain.getNextUnread().getValue() == "b");
        test(!plain.hasUnread());

        test(filtered.getNextUnread().getValue() == "a");
        test(!filtered.hasUnread());
    }

    // Create a sample-filtered reader after the writer queued three samples, one of which makes the
    // filter predicate throw. The writer treats that sample as not matching, so the reader still
    // attaches and is initialized with the other two samples. The reader matches any key, so the
    // key the throwing sample was written for reaches the predicate too.
    {
        Topic<string, string> topic(node, "attachSampleFilterThrow");
        Topic<string, int> barrier(node, "attachSampleFilterThrowBarrier");
        Topic<string, int> done(node, "attachSampleFilterThrowDone");

        // Wait until the writer queued all three samples.
        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "barrier").getNextUnread();

        auto reader = makeAnyKeyReader(topic, Filter<string>("throwOnValue", "boom"), "", config);
        reader.waitForUnread(2);
        auto samples = reader.getAllUnread();
        test(samples.size() == 2);
        test(samples[0].getKey() == "elem");
        test(samples[0].getEvent() == SampleEvent::Add);
        test(samples[0].getValue() == "value1");
        test(samples[1].getKey() == "elem");
        test(samples[1].getEvent() == SampleEvent::Update);
        test(samples[1].getValue() == "value3");

        // Signal the writer that the reader was initialized, so it can tear down.
        auto doneWriter = makeSingleKeyWriter(done, "done");
        doneWriter.waitForReaders();
        doneWriter.update(0);
    }

    // An unfiltered reader and a sample-filtered one on the same writer and key. The writer runs the sample filter
    // while it selects the live destinations, and the predicate throws for one update and for the remove. Those
    // samples are skipped for the filtered reader only; the unfiltered reader receives all of them.
    {
        Topic<string, string> topic(node, "liveSampleFilterThrow");
        Topic<string, int> done(node, "liveSampleFilterThrowDone");

        auto reader = makeSingleKeyReader(topic, "elem", "", config);
        auto filtered = makeSingleKeyReader(topic, "elem", Filter<string>("throwOnValue", "boom"), "", config);

        auto sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Add);
        test(sample.getValue() == "value1");
        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Update);
        test(sample.getValue() == "boom");
        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Update);
        test(sample.getValue() == "value2");
        sample = reader.getNextUnread();
        test(sample.getEvent() == SampleEvent::Remove);

        // The filtered reader skips the two samples whose predicate threw, and nothing else: had "boom" been
        // delivered, it would be this sample instead of "value2".
        sample = filtered.getNextUnread();
        test(sample.getEvent() == SampleEvent::Add);
        test(sample.getValue() == "value1");
        sample = filtered.getNextUnread();
        test(sample.getEvent() == SampleEvent::Update);
        test(sample.getValue() == "value2");
        test(!filtered.hasUnread());

        auto doneWriter = makeSingleKeyWriter(done, "done");
        doneWriter.waitForReaders();
        doneWriter.update(0);
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

    // Two readers subscribing to the same key are distinct reader elements, so each must be initialized with the key's
    // sample exactly once and stay independently subscribed. The live update "valueB", published only after both
    // readers drain their initialization sample, must then be the next sample each sees; a reader initialized with the
    // wrong reader's batch, or twice, would surface a second "valueA" ahead of it.
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

        {
            auto probe = makeSingleKeyReader(topic, "elemA", "", config);
            probe.waitForWriters(1);
        }

        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "barrier").getNextUnread();

        auto reader = makeSingleKeyReader(topic, "elemB", "", config);
        reader.waitForWriters(1); // attached via the empty initialization batch; a live sample is now delivered

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
        // The samples must be delivered in global sample-id order (elemA:1, elemB:2, elemA:3), not grouped by key, so
        // a broken or removed sort in the merge is caught.
        auto samples = reader.getAllUnread();
        test(samples.size() == 3);
        test(samples[0].getKey() == "elemA" && samples[0].getValue() == "valueA1");
        test(samples[1].getKey() == "elemB" && samples[1].getValue() == "valueB1");
        test(samples[2].getKey() == "elemA" && samples[2].getValue() == "valueA2");

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

    // Two same-name topics on this node, each with a single-key reader on a different key of the one multi-key
    // writer. Each reader must receive only its own key's value even though the two topics number their reader
    // elements and keys independently: the initialization is addressed to the exact destination topic, so the
    // colliding ids resolve within the intended topic rather than the other same-name topic.
    {
        Topic<string, string> topicA(node, "sameNameInit");
        Topic<string, string> topicB(node, "sameNameInit");

        // A prior reader on each topic, so the reader under test is the second key element of its topic: with
        // per-topic numbering the two topics then assign it the same element and key id.
        auto firstA = makeSingleKeyReader(topicA, "firstA", "", config);
        auto firstB = makeSingleKeyReader(topicB, "firstB", "", config);

        auto readerA = makeSingleKeyReader(topicA, "elemA", "", config);
        auto readerB = makeSingleKeyReader(topicB, "elemB", "", config);

        auto sampleA = readerA.getNextUnread();
        test(sampleA.getKey() == "elemA");
        test(sampleA.getValue() == "valueA");

        auto sampleB = readerB.getNextUnread();
        test(sampleB.getKey() == "elemB");
        test(sampleB.getValue() == "valueB");

        test(!readerA.hasUnread()); // readerA must not also receive elemB's value
        test(!readerB.hasUnread()); // readerB must not also receive elemA's value
    }

    // The same collision, but the writer has queued both keys before the readers attach, so each reader is
    // initialized from the writer's queue (a non-empty initialization batch) rather than a live update. The
    // initialization must be routed to the exact destination topic, or a reader is delivered the other key's value.
    {
        Topic<string, string> topicA(node, "lateSameNameInit");
        Topic<string, string> topicB(node, "lateSameNameInit");
        Topic<string, int> barrier(node, "lateSameNameInitBarrier");
        Topic<string, int> done(node, "lateSameNameInitDone");

        // Attach each topic to the writer's session and bump its element/key id counter, so the readers under test
        // collide on the same element and key ids across the two same-name topics.
        auto firstA = makeSingleKeyReader(topicA, "firstA", "", config);
        auto firstB = makeSingleKeyReader(topicB, "firstB", "", config);

        // Wait until the writer has queued both keys, so the readers below are initialized from the queue.
        [[maybe_unused]] auto _ = makeSingleKeyReader(barrier, "barrier").getNextUnread();

        auto readerA = makeSingleKeyReader(topicA, "elemA", "", config);
        auto readerB = makeSingleKeyReader(topicB, "elemB", "", config);

        auto sampleA = readerA.getNextUnread();
        test(sampleA.getKey() == "elemA");
        test(sampleA.getValue() == "valueA");

        auto sampleB = readerB.getNextUnread();
        test(sampleB.getKey() == "elemB");
        test(sampleB.getValue() == "valueB");

        test(!readerA.hasUnread());
        test(!readerB.hasUnread());

        auto doneWriter = makeSingleKeyWriter(done, "done");
        doneWriter.waitForReaders();
        doneWriter.update(0);
    }

    // Two same-name topics on this node, each with a sample-filtered reader on the one writer's key. Each reader is
    // the first element of its own topic, so the two topics number them identically. Each reader must receive only
    // the samples its own filter matches: the writer publishes "a", "b" and "ab", and the reader filtering on "a"
    // must see "a" and "ab" while the reader filtering on "b" must see "b" and "ab".
    {
        Topic<string, string> topicA(node, "sameNameSampleFilter");
        Topic<string, string> topicB(node, "sameNameSampleFilter");

        auto readerA = makeSingleKeyReader(topicA, "elem", Filter<string>("contains", "a"), "", config);
        auto readerB = makeSingleKeyReader(topicB, "elem", Filter<string>("contains", "b"), "", config);

        test(readerA.getNextUnread().getValue() == "a");
        test(readerA.getNextUnread().getValue() == "ab");
        test(!readerA.hasUnread());

        test(readerB.getNextUnread().getValue() == "b");
        test(readerB.getNextUnread().getValue() == "ab");
        test(!readerB.hasUnread());
    }
}

DEFINE_TEST(::Reader)
