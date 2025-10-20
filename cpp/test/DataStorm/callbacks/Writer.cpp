// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"

using namespace DataStorm;
using namespace std;

// GCC should allow "shadowing" in lambda expressions but doesn't.
#if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

class Writer : public Test::TestHelper
{
public:
    Writer() : Test::TestHelper(false) {}

    void run(int, char**) override;
};

void ::Writer::run(int argc, char* argv[])
{
    function<void(function<void()> call)> customExecutor = nullptr;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--with-executor") == 0)
        {
            customExecutor = [](const function<void()>& cb) { cb(); };
        }
    }

    NodeOptions options{
        .communicator = Ice::initialize(argc, argv),
        .nodeOwnsCommunicator = true,
        .customExecutor = std::move(customExecutor)};

    Node node{std::move(options)};

    WriterConfig config;
    config.sampleCount = -1; // Unlimited sample count
    config.clearHistory = ClearHistoryPolicy::Never;

    Topic<string, int> controller(node, "controller");
    auto writers = makeSingleKeyWriter(controller, "writers");
    auto readers = makeSingleKeyReader(controller, "readers");
    writers.waitForReaders();

    Topic<string, string> topic(node, "string");

    cout << "testing onSamples... " << flush;
    {
        {
            auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
            writer.add("value1");
            test(readers.getNextUnread().getValue() == 1);
        }
        {
            auto writer = makeSingleKeyWriter(topic, "elem2", "", config);
            test(readers.getNextUnread().getValue() == 2);
            writer.add("value1");
            test(readers.getNextUnread().getValue() == 3);
        }
        {
            auto writer = makeSingleKeyWriter(topic, "elem3", "", config);
            writer.add("value1");
            writer.update("value2");
            writer.remove();
            test(readers.getNextUnread().getValue() == 4);
        }
    }
    cout << "ok" << endl;

    cout << "testing onConnectedKeys... " << flush;
    {
        for (int i = 0; i < 2; ++i)
        {
            {
                auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
                promise<bool> p1, p2, p3;
                writer.onConnectedKeys(
                    [&p1](const vector<string>& keys) { p1.set_value(keys.empty()); },
                    [&p2, &p3](CallbackReason action, const string& key)
                    {
                        if (action == CallbackReason::Connect)
                        {
                            p2.set_value(key == "elem1");
                        }
                        else if (action == CallbackReason::Disconnect)
                        {
                            p3.set_value(key == "elem1");
                        }
                    });
                test(p1.get_future().get());
                writers.update(1);
                test(p2.get_future().get());
                writers.update(2);
                test(p3.get_future().get());
            }
            {
                auto writer = makeSingleKeyWriter(topic, "elem2", "", config);
                writer.waitForReaders();
                promise<bool> p1, p2;
                writer.onConnectedKeys(
                    [&p1](vector<string> keys) { p1.set_value(!keys.empty() && keys[0] == "elem2"); },
                    [&p2](CallbackReason action, const string& key)
                    {
                        if (action == CallbackReason::Connect)
                        {
                            test(false);
                        }
                        else if (action == CallbackReason::Disconnect)
                        {
                            p2.set_value(key == "elem2");
                        }
                    });
                test(p1.get_future().get());
                writers.update(3);
                test(p2.get_future().get());
            }
            {
                test(readers.getNextUnread().getValue() == 1);
                auto writer = makeSingleKeyWriter(topic, "elem3", "", config);
                test(readers.getNextUnread().getValue() == 2);
            }
            {
                auto writer = makeSingleKeyWriter(topic, "elem4", "", config);
                test(readers.getNextUnread().getValue() == 3);
            }

            test(readers.getNextUnread().getValue() == 4);
        }

        {
            auto writer = makeAnyKeyWriter(topic, "", config);
            promise<bool> p1, p2, p3;
            writer.onConnectedKeys(
                [&p1](const vector<string>& keys) { p1.set_value(keys.empty()); },
                [&p2, &p3](CallbackReason action, const string& key)
                {
                    if (action == CallbackReason::Connect)
                    {
                        p2.set_value(key == "anyelem1");
                    }
                    else if (action == CallbackReason::Disconnect)
                    {
                        p3.set_value(key == "anyelem1");
                    }
                });
            test(p1.get_future().get());
            writers.update(1);
            test(p2.get_future().get());
            writers.update(2);
            test(p3.get_future().get());
        }

        writers.update(3);

        {
            test(readers.getNextUnread().getValue() == 1);
            auto writer = makeSingleKeyWriter(topic, "anyelem3", "", config);
            test(readers.getNextUnread().getValue() == 2);
        }

        test(readers.getNextUnread().getValue() == 3);
    }
    cout << "ok" << endl;

    cout << "testing onConnected... " << flush;
    {
        for (int i = 0; i < 2; ++i)
        {
            {
                auto writer = makeSingleKeyWriter(topic, "elem1", "", config);
                promise<bool> p1, p2, p3;
                writer.onConnectedReaders(
                    [&p1](const vector<string>& readers) { p1.set_value(readers.empty()); },
                    [&p2, &p3](CallbackReason action, const string& reader)
                    {
                        if (action == CallbackReason::Connect)
                        {
                            p2.set_value(reader == "reader1");
                        }
                        else if (action == CallbackReason::Disconnect)
                        {
                            p3.set_value(reader == "reader1");
                        }
                    });
                test(p1.get_future().get());
                test(writer.getConnectedReaders().empty());
                writers.update(1);
                test(p2.get_future().get());
                test(writer.getConnectedReaders() == vector<string>{"reader1"});
                writers.update(2);
                test(p3.get_future().get());
            }
            {
                auto writer = makeSingleKeyWriter(topic, "elem2", "", config);
                writer.waitForReaders();
                promise<bool> p1, p2;
                writer.onConnectedReaders(
                    [&p1](vector<string> readers) { p1.set_value(!readers.empty() && readers[0] == "reader2"); },
                    [&p2](CallbackReason action, const string& reader)
                    {
                        if (action == CallbackReason::Connect)
                        {
                            test(false);
                        }
                        else if (action == CallbackReason::Disconnect)
                        {
                            p2.set_value(reader == "reader2");
                        }
                    });
                test(p1.get_future().get());
                writers.update(3);
                test(p2.get_future().get());
            }
            {
                test(readers.getNextUnread().getValue() == 1);
                auto writer = makeSingleKeyWriter(topic, "elem3", "writer1", config);
                test(readers.getNextUnread().getValue() == 2);
            }
            {
                auto writer = makeSingleKeyWriter(topic, "elem4", "writer2", config);
                test(readers.getNextUnread().getValue() == 3);
            }

            test(readers.getNextUnread().getValue() == 4);
        }

        {
            auto writer = makeAnyKeyWriter(topic, "", config);
            promise<bool> p1, p2, p3;
            writer.onConnectedReaders(
                [&p1](const vector<string>& readers) { p1.set_value(readers.empty()); },
                [&p2, &p3](CallbackReason action, const string& reader)
                {
                    if (action == CallbackReason::Connect)
                    {
                        p2.set_value(reader == "reader1");
                    }
                    else if (action == CallbackReason::Disconnect)
                    {
                        p3.set_value(reader == "reader1");
                    }
                });
            test(p1.get_future().get());
            writers.update(1);
            test(p2.get_future().get());
            test(writer.getConnectedReaders() == vector<string>{"reader1"});
            writers.update(2);
            test(p3.get_future().get());
        }

        writers.update(3);

        {
            test(readers.getNextUnread().getValue() == 1);
            auto writer = makeSingleKeyWriter(topic, "anyelem3", "writer1", config);
            test(readers.getNextUnread().getValue() == 2);
        }

        test(readers.getNextUnread().getValue() == 3);
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
