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

    Topic<string, string> topic(node, "stringtopic");
    Topic<string, bool> controller(node, "controller");

    auto readers = makeSingleKeyWriter(controller, "readers");
    auto writers = makeSingleKeyReader(controller, "writers", "", {-1, 0, ClearHistoryPolicy::Never});

    writers.waitForWriters();

    // Writer/reader name
    {
        auto reader = makeSingleKeyReader(topic, "key1", "readername1");
        test(reader.getNextUnread().getOrigin() == "writername1");
        readers.update(true); // Reader is done
    }

    // Writer sample count
    {
        auto read = [&topic, &writers, &readers](int count)
        {
            while (!writers.getNextUnread().getValue())
                ; // Wait for writer to write the samples before reading

            readers.update(false);

            ReaderConfig config;
            config.clearHistory = ClearHistoryPolicy::Never;
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            test(count < 6 || reader.getNextUnread().getValue() == "value1");
            test(count < 5 || reader.getNextUnread().getValue() == "value2");
            test(count < 4 || reader.getNextUnread().getEvent() == SampleEvent::Remove);
            test(count < 3 || reader.getNextUnread().getValue() == "value3");
            test(count < 2 || reader.getNextUnread().getValue() == "value4");
            test(count < 1 || reader.getNextUnread().getEvent() == SampleEvent::Remove);
            readers.update(true); // Reader is done
        };

        read(6); // Writer keeps all the samples
        read(4); // Writer keeps 4 samples
        read(3); // Writer keeps last instance samples
    }

    // Reader sample count
    {
        auto read = [&topic, &writers, &readers](int count, ReaderConfig config)
        {
            while (!writers.getNextUnread().getValue())
                ; // Wait for writer to write the samples before reading

            readers.update(false);
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            test(count < 6 || reader.getNextUnread().getValue() == "value1");
            test(count < 5 || reader.getNextUnread().getValue() == "value2");
            test(count < 4 || reader.getNextUnread().getEvent() == SampleEvent::Remove);
            test(count < 3 || reader.getNextUnread().getValue() == "value3");
            test(count < 2 || reader.getNextUnread().getValue() == "value4");
            test(count < 1 || reader.getNextUnread().getEvent() == SampleEvent::Remove);
            readers.update(true); // Reader is done
        };

        // Keep all the samples in the history.
        {
            ReaderConfig config;
            config.clearHistory = ClearHistoryPolicy::Never;
            read(6, config);
        }

        // Keep 4 samples in the history
        {
            ReaderConfig config;
            config.clearHistory = ClearHistoryPolicy::Never;
            config.sampleCount = 4;
            read(4, config);
        }

        // Keep last instance samples in the history
        {
            ReaderConfig config;
            config.clearHistory = ClearHistoryPolicy::OnAdd;
            read(3, config);
        }
    }

    // Writer sample lifetime
    {
        while (!writers.getNextUnread().getValue())
            ; // Wait for writer to write the samples before reading

        // Writer keeps 3ms worth of samples
        readers.update(false);
        ReaderConfig config;
        config.clearHistory = ClearHistoryPolicy::Never;
        auto reader = makeSingleKeyReader(topic, "elem1", "", config);
        test(reader.getNextUnread().getValue() == "value3");
        test(reader.getNextUnread().getValue() == "value4");
        test(reader.getNextUnread().getEvent() == SampleEvent::Remove);
        readers.update(true); // Reader is done
    }

    // Reader sample lifetime
    {
        while (!writers.getNextUnread().getValue())
            ; // Wait for writer to write the samples before reading

        ReaderConfig config;
        config.sampleLifetime = 390;
        config.clearHistory = ClearHistoryPolicy::Never;

        auto now = chrono::system_clock::now();

        // Reader wants 390ms worth of samples
        readers.update(false);
        auto reader = makeSingleKeyReader(topic, "elem1", "", config);
        reader.waitForUnread(3);
        auto samples = reader.getAllUnread();
        test(samples[0].getValue() == "value3");
        test(samples[1].getValue() == "value4");
        test(samples[2].getEvent() == SampleEvent::Remove);
        readers.update(true); // Reader is done

        for (const auto& s : samples)
        {
            test(s.getTimeStamp() >= (now - chrono::milliseconds(150)));
        }
    }

    // Writer clearHistory
    {
        topic.setUpdater<string>("concat", [](string& value, const string& update) { value += update; });
        ReaderConfig config;
        config.clearHistory = ClearHistoryPolicy::Never;

        {
            while (!writers.getNextUnread().getValue())
                ; // Wait for writer to write the samples before reading
            readers.update(false);
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            reader.waitForUnread(22);
            readers.update(true); // Reader is done
        }

        {
            while (!writers.getNextUnread().getValue())
                ; // Wait for writer to write the samples before reading
            readers.update(false);
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            reader.waitForUnread(2);
            test(reader.getNextUnread().getValue() == "value3");
            test(reader.getNextUnread().getValue() == "value4");
            readers.update(true); // Reader is done
        }

        {
            while (!writers.getNextUnread().getValue())
                ; // Wait for writer to write the samples before reading
            readers.update(false);
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            reader.waitForUnread(3);
            test(reader.getNextUnread().getEvent() == DataStorm::SampleEvent::Remove);
            test(reader.getNextUnread().getValue() == "value3");
            test(reader.getNextUnread().getValue() == "value4");
            readers.update(true); // Reader is done
        }

        {
            while (!writers.getNextUnread().getValue())
                ; // Wait for writer to write the samples before reading
            readers.update(false);
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            reader.waitForUnread(1);
            test(reader.getNextUnread().getValue() == "value4");
            readers.update(true); // Reader is done
        }

        {
            while (!writers.getNextUnread().getValue())
                ; // Wait for writer to write the samples before reading
            readers.update(false);
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            reader.waitForUnread(4);
            test(reader.getNextUnread().getValue() == "value");
            auto s = reader.getNextUnread();
            test(s.getEvent() == SampleEvent::PartialUpdate);
            test(s.getUpdateTag() == "concat");
            test(s.getValue() == "value1");
            test(reader.getNextUnread().getValue() == "value12");
            test(reader.getNextUnread().getValue() == "value123");
            readers.update(true); // Reader is done
        }
    }

    // Reader clearHistory
    {
        while (!writers.getNextUnread().getValue())
            ; // Wait for writer to write the samples before reading
        readers.update(false);

        {
            ReaderConfig config;
            config.clearHistory = ClearHistoryPolicy::Never;
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            reader.waitForUnread(9);
        }

        {
            ReaderConfig config;
            config.clearHistory = ClearHistoryPolicy::OnAdd;
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            reader.waitForUnread(5);
        }

        {
            ReaderConfig config;
            config.clearHistory = ClearHistoryPolicy::OnRemove;
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            reader.waitForUnread(6);
        }

        {
            ReaderConfig config;
            config.clearHistory = ClearHistoryPolicy::OnAll;
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            reader.waitForUnread(1);
        }

        {
            ReaderConfig config;
            config.clearHistory = ClearHistoryPolicy::OnAllExceptPartialUpdate;
            auto reader = makeSingleKeyReader(topic, "elem1", "", config);
            reader.waitForUnread(4);
        }

        readers.update(true); // Reader is done
    }

    // Reader priority discard policy
    {
        readers.update(false);

        ReaderConfig config;
        config.clearHistory = ClearHistoryPolicy::Never;
        config.discardPolicy = DiscardPolicy::Priority;
        auto reader = makeSingleKeyReader(topic, "elemdp1", "", config);
        test(reader.getNextUnread().getValue() == "value1");
        test(reader.getNextUnread().getValue() == "value2");
        test(reader.getNextUnread().getValue() == "value21");
        test(reader.getNextUnread().getEvent() == DataStorm::SampleEvent::Remove);

        readers.update(true); // Reader is done
    }
    {
        readers.update(false);

        ReaderConfig config;
        config.clearHistory = ClearHistoryPolicy::Never;
        config.discardPolicy = DiscardPolicy::Priority;
        auto reader = makeSingleKeyReader(topic, "elemdp2", "", config);
        test(reader.getNextUnread().getValue() == "value1");
        test(reader.getNextUnread().getValue() == "value2");
        test(reader.getNextUnread().getValue() == "value21");
        test(reader.getNextUnread().getEvent() == DataStorm::SampleEvent::Remove);

        readers.update(true); // Reader is done
    }

    // Reader sending discard policy
    {
        while (!writers.getNextUnread().getValue())
            ; // Wait for writer to write the samples before reading

        readers.update(false);
        while (true)
        {
            auto lastUnread = chrono::time_point<chrono::system_clock>::min();
            Topic<string, int> topic1(node, "sendTimeTopic");
            ReaderConfig config;
            config.clearHistory = ClearHistoryPolicy::Never;
            config.discardPolicy = DiscardPolicy::SendTime;
            auto reader = makeSingleKeyReader(topic1, "elem", "", config);
            int writerCount = 10;
            int sampleCount = 0;
            while (true)
            {
                auto sample = reader.getNextUnread();
                test(sample.getTimeStamp() > lastUnread);
                lastUnread = sample.getTimeStamp();
                ++sampleCount;
                if (sample.getValue() == (writerCount - 1))
                {
                    break;
                }
            }
            if (sampleCount < writerCount)
            {
                test(!reader.hasUnread());
                break;
            }
        }
        readers.update(true); // Reader is done
    }
}

DEFINE_TEST(::Reader)
