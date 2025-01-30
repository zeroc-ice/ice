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
