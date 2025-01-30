// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "Test.h"
#include "TestHelper.h"

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

    cout << "testing node..." << flush;
    {
        {
            Node n;
            Node nm(std::move(n));
            auto nm2 = std::move(nm);
            [[maybe_unused]] Ice::CommunicatorPtr communicator = nm2.getCommunicator();
            [[maybe_unused]] Ice::ConnectionPtr connection = nm2.getSessionConnection("s");
        }

        {
            // Communicators shared with DataStorm must have a property set that can use the "DataStorm" opt-in prefix.
            Ice::InitializationData initData;
            initData.properties = make_shared<Ice::Properties>(vector<string>{"DataStorm"});
            Ice::CommunicatorHolder communicatorHolder{Ice::initialize(initData)};
            Node n2{communicatorHolder.communicator()};
        }

        Ice::InitializationData initData;
        initData.properties = make_shared<Ice::Properties>(vector<string>{"DataStorm"});
        auto c = Ice::initialize(initData);
        {
            Node n22(c);
        }
        {
            const Ice::CommunicatorPtr& c2 = c;
            Node n23(c2);
        }
        {
            const Ice::CommunicatorPtr& c3 = c;
            Node n24(c3);
        }
        {
            Ice::CommunicatorPtr& c4 = c;
            Node n25(c4);
        }
        c->destroy();

        Node n3;

        try
        {
            Node n4(argc, argv, "config.file");
        }
        catch (const Ice::FileException&)
        {
        }

        Node n5(argc, argv);

        {
            Node n6;
            test(!n6.isShutdown());
            n6.shutdown();
            test(n6.isShutdown());
            n6.waitForShutdown();

            auto testException = [](const function<void()>& fn)
            {
                try
                {
                    fn();
                    test(false);
                }
                catch (NodeShutdownException&)
                {
                }
                catch (...)
                {
                    test(false);
                }
            };

            Topic<int, string> t1(n6, "t1");
            testException([&t1]() { t1.waitForWriters(); });
            testException([&t1]() { t1.waitForNoWriters(); });
            testException([&t1]() { t1.waitForReaders(); });
            testException([&t1]() { t1.waitForNoReaders(); });

            auto writer = makeSingleKeyWriter(t1, 0);
            testException([&writer] { writer.waitForReaders(); });
            testException([&writer] { writer.waitForNoReaders(); });

            auto reader = makeSingleKeyReader(t1, 0);
            testException([&reader] { reader.waitForWriters(); });
            testException([&reader] { reader.waitForNoWriters(); });
            testException([&reader] { reader.waitForUnread(); });
            testException([&reader] { [[maybe_unused]] auto _ = reader.getNextUnread(); });
        }
    }
    cout << "ok" << endl;

    cout << "testing topic... " << flush;
    {
        Topic<int, string> t1(node, "t1");
        Topic<int, string> t2(node, "t2");
        Topic<StructKey, string> t3(node, "t3");

        // TODO fix class cloner
        // Topic<ClassKey, string> t4(node, "t4");

        Topic<int, string>::KeyType k1 = 5;
        Topic<int, string>::ValueType v1("string");
        Topic<int, string>::UpdateTagType tag1("string");

        Topic<int, string>::WriterType* writer = nullptr;
        if (writer != nullptr)
        {
            test(writer->getConnectedKeys()[0] == k1); // Use variables to prevent unused variable warnings
        }
        Topic<int, string>::ReaderType* reader = nullptr;
        if (reader != nullptr)
        {
            [[maybe_unused]] auto _ = reader->getConnectedKeys();
        }

        auto tc1 = std::move(t1);

        test(!tc1.hasWriters());
        tc1.waitForWriters(0);
        tc1.waitForNoWriters();

        test(!t2.hasReaders());
        t2.waitForReaders(0);
        t2.waitForNoReaders();

        tc1.setWriterDefaultConfig(WriterConfig());
        t2.setReaderDefaultConfig(ReaderConfig());

        tc1.setUpdater<string>("test", [](string&, const string&) {});
    }
    cout << "ok" << endl;

    cout << "testing writer... " << flush;
    {
        Topic<string, string> topic(node, "topic");

        auto testWriter = [](Topic<string, string>::WriterType& writer)
        {
            [[maybe_unused]] bool hasReaders = writer.hasReaders();
            writer.waitForReaders(0);
            writer.waitForNoReaders();
            [[maybe_unused]] auto _ = writer.getConnectedReaders();
            [[maybe_unused]] auto connectedKeys = writer.getConnectedKeys();
            test(writer.getAll().empty());
            try
            {
                [[maybe_unused]] auto last = writer.getLast();
            }
            catch (const std::logic_error&)
            {
            }
            [[maybe_unused]] auto all = writer.getAll();
            writer.onConnectedKeys([](const vector<string>&) {}, [](CallbackReason, const string&) {});
        };

        auto skw = makeSingleKeyWriter(topic, "key");
        skw = makeSingleKeyWriter(topic, "key", "", WriterConfig());
        SingleKeyWriter<string, string> skw1(topic, "key");

        auto skwm = std::move(skw);
        testWriter(skwm);
        skwm.add("test");
        skwm.update(string("test"));
        skwm.partialUpdate<int>("updatetag")(10);
        skwm.remove();

        auto skws = make_shared<SingleKeyWriter<string, string>>(topic, "key");
        skws = make_shared<SingleKeyWriter<string, string>>(topic, "key", "", WriterConfig());

        auto mkw = makeMultiKeyWriter(topic, {"key"});
        mkw = makeMultiKeyWriter(topic, {"key"}, "", WriterConfig());
        MultiKeyWriter<string, string> mkw1(topic, {"key"});

        auto mkwm = std::move(mkw);
        testWriter(mkwm);
        mkwm.add("key", "test");
        mkwm.update("key", string("test"));
        mkwm.partialUpdate<int>("updatetag")("key", 10);
        mkwm.remove("key");

        auto mkws = make_shared<MultiKeyWriter<string, string>>(topic, vector<string>{"key"});
        mkws = make_shared<MultiKeyWriter<string, string>>(topic, vector<string>{"key"}, "", WriterConfig());

        auto akw = makeAnyKeyWriter(topic);
        akw = makeAnyKeyWriter(topic, "", WriterConfig());
        MultiKeyWriter<string, string> akw1(topic, {});

        auto akwm = std::move(akw);
        testWriter(akwm);

        auto akws = make_shared<MultiKeyWriter<string, string>>(topic, vector<string>{});
        akws = make_shared<MultiKeyWriter<string, string>>(topic, vector<string>{}, "", WriterConfig());
    }
    cout << "ok" << endl;

    cout << "testing reader... " << flush;
    {
        Topic<string, string> topic(node, "topic");

        auto testReader = [](Topic<string, string>::ReaderType& reader)
        {
            [[maybe_unused]] bool hasWriters = reader.hasWriters();
            reader.waitForWriters(0);
            reader.waitForNoWriters();
            [[maybe_unused]] auto _ = reader.getConnectedWriters();
            [[maybe_unused]] auto connectedKeys = reader.getConnectedKeys();
            [[maybe_unused]] auto allUnread = reader.getAllUnread();
            reader.waitForUnread(0);
            [[maybe_unused]] bool hasUnread = reader.hasUnread();
            reader.onConnectedKeys([](const vector<string>&) {}, [](CallbackReason, const string&) {});
            reader.onSamples([](const vector<Sample<string, string>>&) {}, [](const Sample<string, string>&) {});
        };

        auto skr = makeSingleKeyReader(topic, "key");
        skr = makeSingleKeyReader(topic, "key", "", ReaderConfig());
        testReader(skr);
        auto skrsf = makeSingleKeyReader(topic, "key", Filter<string>("_regex", ".*"));
        skrsf = makeSingleKeyReader(topic, "key", Filter<string>("_regex", ".*"), "", ReaderConfig());

        auto mkr = makeMultiKeyReader(topic, {"key"});
        mkr = makeMultiKeyReader(topic, {"key"}, "", ReaderConfig());
        testReader(mkr);
        auto mkrsf = makeMultiKeyReader(topic, {"key"}, Filter<string>("_regex", ".*"));
        mkrsf = makeMultiKeyReader(topic, {"key"}, Filter<string>("_regex", ".*"), "", ReaderConfig());

        auto akr = makeAnyKeyReader(topic);
        akr = makeAnyKeyReader(topic, "", ReaderConfig());
        testReader(akr);
        auto akrsf = makeAnyKeyReader(topic, Filter<string>("_regex", ".*"));
        akrsf = makeAnyKeyReader(topic, Filter<string>("_regex", ".*"), "", ReaderConfig());

        auto fr = makeFilteredKeyReader(topic, Filter<string>(string("_regex"), string(".*")));
        fr = makeFilteredKeyReader(topic, Filter<string>("_regex", ".*"), "", ReaderConfig());
        testReader(fr);
        auto frsf = makeFilteredKeyReader(topic, Filter<string>("_regex", ".*"), Filter<string>("_regex", ".*"));
        frsf = makeFilteredKeyReader(
            topic,
            Filter<string>("_regex", ".*"),
            Filter<string>("_regex", ".*"),
            "",
            ReaderConfig());

        auto skrs = make_shared<SingleKeyReader<string, string>>(topic, "key");
        skrs = make_shared<SingleKeyReader<string, string>>(topic, "key", "", ReaderConfig());

        auto mkrs = make_shared<MultiKeyReader<string, string>>(topic, vector<string>{"key"});
        mkrs = make_shared<MultiKeyReader<string, string>>(topic, vector<string>{"key"}, "", ReaderConfig());

        auto akrs = make_shared<MultiKeyReader<string, string>>(topic, vector<string>{});
        akrs = make_shared<MultiKeyReader<string, string>>(topic, vector<string>{}, "", ReaderConfig());

        auto frs = make_shared<FilteredKeyReader<string, string>>(topic, Filter<string>("_regex", ".*"));
        frs = make_shared<FilteredKeyReader<string, string>>(topic, Filter<string>("_regex", ".*"), "", ReaderConfig());

        try
        {
            [[maybe_unused]] auto unknownFilteredReader = makeFilteredKeyReader(topic, Filter<string>("unknown", ""));
            test(false);
        }
        catch (const std::invalid_argument&)
        {
        }

        try
        {
            [[maybe_unused]] auto regexFilteredReader = makeFilteredKeyReader(topic, Filter<string>("_regex", "("));
            test(false);
        }
        catch (const std::invalid_argument&)
        {
        }
        catch (const std::regex_error&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing sample... " << flush;
    {
        Topic<string, string> topic(node, "topic");
        auto skw = makeSingleKeyWriter(topic, "key");
        skw.add("test");
        test(skw.getLast().getKey() == "key");
        test(skw.getLast().getValue() == "test");
        test(skw.getLast().getEvent() == SampleEvent::Add);
        skw.update("test2");
        test(skw.getLast().getKey() == "key");
        test(skw.getLast().getValue() == "test2");
        test(skw.getLast().getEvent() == SampleEvent::Update);
        skw.remove();
        test(skw.getLast().getKey() == "key");
        test(skw.getLast().getValue() == "");
        test(skw.getLast().getEvent() == SampleEvent::Remove);
        skw.partialUpdate<string>("partialupdate")("update");
        test(skw.getLast().getKey() == "key");
        test(skw.getLast().getValue() == "");
        test(skw.getLast().getUpdateTag() == "partialupdate");
        test(skw.getLast().getEvent() == SampleEvent::PartialUpdate);

        ostringstream os;
        os << skw.getLast();
        os << skw.getLast().getEvent();
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
