// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

#include <thread>

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

    Topic<string, string> topic(node, "stringtopic");
    Topic<string, bool> controller(node, "controller");

    auto writers = makeSingleKeyWriter(controller, "writers");
    auto readers = makeSingleKeyReader(controller, "readers", "", {-1, 0, ClearHistoryPolicy::Never});

    {
        WriterConfig config;
        config.clearHistory = ClearHistoryPolicy::Never;
        topic.setWriterDefaultConfig(config);
    }

    cout << "testing writer/reader name... " << flush;
    {
        auto writer = makeSingleKeyWriter(topic, "key1", "writername1");
        writer.update("update");
        while (!readers.getNextUnread().getValue())
            ; // Wait for reader to be done
    }
    cout << "ok" << endl;

    cout << "testing writer sampleCount... " << flush;
    {
        auto write = [&topic, &writers, &readers](WriterConfig config)
        {
            writers.update(false); // Not ready
            auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
            writer.add("value1");
            writer.update("value2");
            writer.remove();
            writer.add("value3");
            writer.update("value4");
            writer.remove();
            writers.update(true); // Ready

            while (!readers.getNextUnread().getValue())
                ; // Wait for reader to be done
        };

        // Keep all the samples in the history.
        {
            WriterConfig config;
            config.clearHistory = ClearHistoryPolicy::Never;
            write(config);
        }

        // Keep 4 samples in the history
        {
            WriterConfig config;
            config.sampleCount = 4;
            write(config);
        }

        // Keep last instance samples in the history
        {
            WriterConfig config;
            config.clearHistory = ClearHistoryPolicy::OnAdd;
            write(config);
        }
    }
    cout << "ok" << endl;

    cout << "testing reader sampleCount... " << flush;
    {
        auto write = [&topic, &writers, &readers]()
        {
            writers.update(false); // Not ready
            auto writer = makeSingleKeyWriter(topic, "elem1");
            writer.add("value1");
            writer.update("value2");
            writer.remove();
            writer.add("value3");
            writer.update("value4");
            writer.remove();
            writers.update(true); // Ready

            while (!readers.getNextUnread().getValue())
                ; // Wait for reader to be done
        };

        write(); // Reader keeps all the samples in the history.
        write(); // Reader keeps 4 samples in the history.
        write(); // Reader keeps last instance samples in the history.
    }
    cout << "ok" << endl;

    cout << "testing writer sampleLifetime... " << flush;
    {
        writers.update(false); // Not ready

        // Keep 3ms worth of samples in the history
        WriterConfig config;
        config.sampleLifetime = 20;
        auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
        writer.add("value1");
        writer.update("value2");
        writer.remove();
        this_thread::sleep_for(chrono::milliseconds(20));
        writers.update(true); // Ready
        writer.waitForReaders();
        writer.add("value3");
        writer.update("value4");
        writer.remove();

        while (!readers.getNextUnread().getValue())
            ; // Wait for reader to be done
    }
    cout << "ok" << endl;

    cout << "testing reader sampleLifetime... " << flush;
    {
        writers.update(false); // Not ready
        auto writer = makeSingleKeyWriter(topic, "elem1");
        writer.add("value1");
        writer.update("value2");
        writer.remove();
        this_thread::sleep_for(chrono::milliseconds(400));
        writer.add("value3");
        writer.update("value4");
        writer.remove();
        writers.update(true); // Ready

        while (!readers.getNextUnread().getValue())
            ; // Wait for reader to be done
    }
    cout << "ok" << endl;

    cout << "testing writer clearHistory... " << flush;
    {
        topic.setUpdater<string>("concat", [](string& value, const string& update) { value += update; });

        {
            writers.update(false); // Not ready
            WriterConfig config;
            config.clearHistory = ClearHistoryPolicy::Never;
            auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
            writer.add("value1");
            for (int i = 0; i < 20; ++i)
            {
                writer.update("value2");
            }
            writer.remove();
            test(writer.getAll().size() == 22);
            writers.update(true); // Ready
            while (!readers.getNextUnread().getValue())
                ; // Wait for reader to be done
        }

        {
            writers.update(false); // Not ready
            WriterConfig config;
            config.clearHistory = ClearHistoryPolicy::OnAdd;
            auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
            writer.add("value1");
            writer.update("value2");
            writer.remove();
            writer.add("value3");
            writer.update("value4");
            test(writer.getAll().size() == 2);
            writers.update(true); // Ready
            while (!readers.getNextUnread().getValue())
                ; // Wait for reader to be done
        }

        {
            writers.update(false); // Not ready
            WriterConfig config;
            config.clearHistory = ClearHistoryPolicy::OnRemove;
            auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
            writer.add("value1");
            writer.update("value2");
            writer.remove();
            writer.add("value3");
            writer.update("value4");
            test(writer.getAll().size() == 3);
            writers.update(true); // Ready
            while (!readers.getNextUnread().getValue())
                ; // Wait for reader to be done
        }

        {
            writers.update(false); // Not ready
            WriterConfig config;
            config.clearHistory = ClearHistoryPolicy::OnAll;
            auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
            writer.add("value1");
            writer.update("value2");
            writer.remove();
            writer.add("value3");
            writer.update("value4");
            test(writer.getAll().size() == 1);
            writers.update(true); // Ready
            while (!readers.getNextUnread().getValue())
                ; // Wait for reader to be done
        }

        {
            writers.update(false); // Not ready
            WriterConfig config;
            config.clearHistory = ClearHistoryPolicy::OnAllExceptPartialUpdate;
            auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
            writer.add("value1");
            writer.update("value2");
            writer.partialUpdate<string>("concat")("1");
            writer.remove();
            writer.add("value3");
            writer.update("value");
            writer.partialUpdate<string>("concat")("1");
            writer.partialUpdate<string>("concat")("2");
            writer.partialUpdate<string>("concat")("3");
            test(writer.getAll().size() == 4);
            test(writer.getAll()[1].getValue() == "value1");
            writers.update(true); // Ready
            while (!readers.getNextUnread().getValue())
                ; // Wait for reader to be done
        }
    }
    cout << "ok" << endl;

    cout << "testing reader clearHistory... " << flush;
    {
        writers.update(false); // Not ready
        WriterConfig config;
        config.clearHistory = ClearHistoryPolicy::Never;
        auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
        writer.add("value1");
        writer.update("value2");
        writer.partialUpdate<string>("concat")("1");
        writer.remove();
        writer.add("value3");
        writer.update("value");
        writer.partialUpdate<string>("concat")("1");
        writer.partialUpdate<string>("concat")("2");
        writer.partialUpdate<string>("concat")("3");
        writers.update(true); // Ready

        while (!readers.getNextUnread().getValue())
            ; // Wait for reader to be done
    }
    cout << "ok" << endl;

    cout << "testing priority discard policy... " << flush;
    {
        WriterConfig config;
        config.priority = 10;
        auto writer1 = makeSingleKeyWriter(topic, "elemdp1", "", config);
        config.priority = 1;
        auto writer2 = makeSingleKeyWriter(topic, "elemdp1", "", config);

        // Wait for the reader to connect to both writers, then send the data.
        writer1.waitForReaders();
        writer2.waitForReaders();

        writer1.add("value1");
        writer2.add("novalue1");
        writer1.update("value2");
        writer2.update("novalue2");
        writer1.partialUpdate<string>("concat")("1");
        writer2.partialUpdate<string>("concat")("no1");
        writer1.remove();
        writer2.remove();

        while (!readers.getNextUnread().getValue())
            ; // Wait for reader to be done
    }
    {
        WriterConfig config;
        config.priority = 1;
        auto writer1 = makeSingleKeyWriter(topic, "elemdp2", "", config);
        config.priority = 10;
        auto writer2 = makeSingleKeyWriter(topic, "elemdp2", "", config);

        writer1.waitForReaders();
        writer2.waitForReaders();

        writer1.add("novalue1");
        writer1.update("novalue2");
        writer1.partialUpdate<string>("concat")("no1");
        writer1.remove();

        writer2.add("value1");
        writer2.update("value2");
        writer2.partialUpdate<string>("concat")("1");
        writer2.remove();

        while (!readers.getNextUnread().getValue())
            ; // Wait for reader to be done
    }
    cout << "ok" << endl;

    cout << "testing send time discard policy... " << flush;
    {
        struct WriterHolder
        {
            Ice::CommunicatorHolder communicator;
            Node node;
            Topic<string, int> topic;
            SingleKeyWriter<string, int> writer;
        };

        writers.update(false); // Not ready
        vector<WriterHolder> w;
        size_t writerCount = 10;
        for (size_t i = 0; i < writerCount; ++i)
        {
            Ice::InitializationData initData;
            initData.properties = node.getCommunicator()->getProperties()->clone();
            Ice::CommunicatorHolder holder(initData);
            Node node1(holder.communicator());
            Topic<string, int> topic1(node1, "sendTimeTopic");
            auto singleKeyWriter = makeSingleKeyWriter(topic1, "elem");
            w.push_back(
                WriterHolder{std::move(holder), std::move(node1), std::move(topic1), std::move(singleKeyWriter)});
        }

        int i = 0;
        for (auto& writerHolder : w)
        {
            this_thread::sleep_for(chrono::microseconds(200));
            writerHolder.writer.update(i++);
        }
        writers.update(true); // Ready

        while (!readers.getNextUnread().getValue())
            ; // Wait for reader to be done
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
