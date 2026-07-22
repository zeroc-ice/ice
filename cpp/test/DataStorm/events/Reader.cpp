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

    // Two same-name topics on this node, each with a single-key reader on a different key of the one multi-key
    // writer. Each reader receives only its own key's value. The reader element id and key id are drawn from a
    // node-wide counter, so the two readers stay distinct across the same-name topics; per-topic ids gave both
    // readers the same element and key id, and the writer's samples were delivered to the wrong reader.
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
    }
}

DEFINE_TEST(::Reader)
