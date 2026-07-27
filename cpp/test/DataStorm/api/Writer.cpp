// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "DataStorm/InternalT.h"
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

namespace
{
    // A user type whose codec specializations provide only the communicator-less (one-arg) encode/decode form. The
    // reader and writer API adapt such codecs to their two-arg internal calls through DataStormI::EncoderT and
    // DataStormI::DecoderT; routing this type through the sample-filter, partial-update, and updater paths below
    // exercises that dispatch.
    struct CustomValue
    {
        int value = 0;
    };
}

namespace DataStorm
{
    template<> struct Encoder<CustomValue>
    {
        static Ice::ByteSeq encode(const CustomValue& value) { return {static_cast<std::byte>(value.value)}; }
    };

    template<> struct Decoder<CustomValue>
    {
        static CustomValue decode(const Ice::ByteSeq& data) { return CustomValue{static_cast<int>(data[0])}; }
    };
}

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

            // A malformed session identity is fail-soft.
            test(nm2.getSessionConnection("a/b/c") == nullptr);

            // The shutdown members are safe on a moved-from node; a moved-from node reports itself as shut down.
            // NOLINTBEGIN(clang-analyzer-cplusplus.Move)
            n.shutdown();
            test(n.isShutdown());
            n.waitForShutdown();
            // NOLINTEND(clang-analyzer-cplusplus.Move)
        }

        {
            Ice::CommunicatorPtr communicator = Ice::initialize();
            Ice::CommunicatorHolder communicatorHolder{communicator};
            Node n2{communicator};
        }

        {
            auto makeInitData = [](const string& property, const string& value)
            {
                Ice::InitializationData initData;
                initData.properties = Ice::createProperties();
                initData.properties->setProperty(property, value);
                return initData;
            };

            // A node configured with an invalid DataStorm.Node.ConnectTo endpoint surfaces a catchable exception
            // from the Node constructor, with and without communicator ownership.
            {
                Ice::CommunicatorHolder holder{Ice::initialize(makeInitData("DataStorm.Node.ConnectTo", "invalid"))};
                try
                {
                    Node n7{holder.communicator()};
                    test(false);
                }
                catch (const Ice::ParseException&)
                {
                }
            }
            try
            {
                Node n8{NodeOptions{
                    .communicator = Ice::initialize(makeInitData("DataStorm.Node.ConnectTo", "invalid")),
                    .nodeOwnsCommunicator = true}};
                test(false);
            }
            catch (const Ice::ParseException&)
            {
            }

            // An invalid DataStorm.Topic.* property value surfaces a catchable exception from the Node constructor
            // rather than from the noexcept Topic methods that lazily create the topic reader or writer.
            {
                Ice::CommunicatorHolder holder{Ice::initialize(makeInitData("DataStorm.Topic.ClearHistory", "onAdd"))};
                try
                {
                    Node n9{holder.communicator()};
                    test(false);
                }
                catch (const Ice::ParseException&)
                {
                }
            }

            // A non-numeric topic default surfaces the property exception from the Node constructor as well.
            {
                Ice::CommunicatorHolder holder{Ice::initialize(makeInitData("DataStorm.Topic.SampleCount", "abc"))};
                try
                {
                    Node n10{holder.communicator()};
                    test(false);
                }
                catch (const Ice::PropertyException&)
                {
                }
            }

            // Retry properties are parsed before the DataStorm adapters are created, leaving a user-supplied
            // communicator reusable when parsing fails.
            {
                Ice::CommunicatorHolder holder{Ice::initialize(makeInitData("DataStorm.Node.RetryDelay", "invalid"))};
                try
                {
                    Node n11{holder.communicator()};
                    test(false);
                }
                catch (const Ice::PropertyException&)
                {
                }

                test(holder.communicator()->getDefaultObjectAdapter() == nullptr);
                holder.communicator()->getProperties()->setProperty("DataStorm.Node.RetryDelay", "500");
                Node n12{holder.communicator()};
            }

            // An adapter creation failure after the default adapter is set also leaves the communicator reusable.
            // Reconstructing a node verifies that the named adapter from the failed construction was destroyed.
            {
                Ice::CommunicatorHolder holder{
                    Ice::initialize(makeInitData("DataStorm.Node.Multicast.Endpoints", "invalid"))};
                try
                {
                    Node n13{holder.communicator()};
                    test(false);
                }
                catch (const invalid_argument&)
                {
                }

                test(holder.communicator()->getDefaultObjectAdapter() == nullptr);
                holder.communicator()->getProperties()->setProperty("DataStorm.Node.Multicast.Endpoints", "");
                Node n14{holder.communicator()};
            }

            // "None" is accepted as an alias for the "Never" discard policy, matching the DiscardPolicy::None
            // enumerator.
            {
                Ice::CommunicatorHolder holder{Ice::initialize(makeInitData("DataStorm.Topic.DiscardPolicy", "None"))};
                Node n15{holder.communicator()};
                Topic<int, string> topic(n15, "discardPolicyAlias");
                auto reader = makeSingleKeyReader(topic, 0);
            }

            // A DataStorm.Node.Multicast.Proxy value must carry the lookup object identity; any other identity is
            // rejected by the Node constructor.
            {
                Ice::CommunicatorHolder holder{Ice::initialize(makeInitData(
                    "DataStorm.Node.Multicast.Proxy",
                    "DataStorm/CustomLookup -d:udp -h 239.255.0.1 -p 10000"))};
                try
                {
                    Node n16{holder.communicator()};
                    test(false);
                }
                catch (const Ice::PropertyException&)
                {
                }

                test(holder.communicator()->getDefaultObjectAdapter() == nullptr);
                holder.communicator()->getProperties()->setProperty(
                    "DataStorm.Node.Multicast.Proxy",
                    "DataStorm/Lookup2 -d:udp -h 239.255.0.1 -p 10000");
                Node n17{holder.communicator()};
            }
        }

        Node n3;

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

        // A communicator-less update codec goes through the DecoderT dispatcher in the registered updater.
        tc1.setUpdater<CustomValue>("customtag", [](string&, CustomValue) {});
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
        // A communicator-less update codec goes through the EncoderT dispatcher when publishing the partial update.
        skwm.partialUpdate<CustomValue>("customtag")(CustomValue{5});
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
        mkwm.partialUpdate<CustomValue>("customtag")("key", CustomValue{5});
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
        // A communicator-less sample-filter codec goes through the EncoderT dispatcher when encoding the criteria.
        skrsf = makeSingleKeyReader(topic, "key", Filter<CustomValue>("customFilter", CustomValue{1}));

        auto mkr = makeMultiKeyReader(topic, {"key"});
        mkr = makeMultiKeyReader(topic, {"key"}, "", ReaderConfig());
        testReader(mkr);
        auto mkrsf = makeMultiKeyReader(topic, {"key"}, Filter<string>("_regex", ".*"));
        mkrsf = makeMultiKeyReader(topic, {"key"}, Filter<string>("_regex", ".*"), "", ReaderConfig());
        mkrsf = makeMultiKeyReader(topic, {"key"}, Filter<CustomValue>("customFilter", CustomValue{1}));

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
        frsf = makeFilteredKeyReader(
            topic,
            Filter<string>("_regex", ".*"),
            Filter<CustomValue>("customFilter", CustomValue{1}));

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
        // A partial update requires the key to have a current value; after the remove it has none, so publishing a
        // partial update throws and the remove stays the last sample.
        try
        {
            skw.partialUpdate<string>("partialupdate")("update");
            test(false);
        }
        catch (const std::logic_error&)
        {
        }
        test(skw.getLast().getEvent() == SampleEvent::Remove);
        // A new full value makes the key updatable again.
        skw.add("test3");
        skw.partialUpdate<string>("partialupdate")("update");
        test(skw.getLast().getKey() == "key");
        test(skw.getLast().getValue() == "test3"); // no updater is registered: the previous value carries over
        test(skw.getLast().getUpdateTag() == "partialupdate");
        test(skw.getLast().getEvent() == SampleEvent::PartialUpdate);

        ostringstream os;
        os << skw.getLast();
        os << skw.getLast().getEvent();
    }
    cout << "ok" << endl;

    cout << "testing element factory cleanup... " << flush;
    {
        // The factory interns elements by value; an element's entry must be erased when the last reference to the
        // element goes away, so a value that is never used again does not keep a map entry alive.
        struct TestKeyFactory : DataStormI::AbstractFactoryT<int, DataStormI::KeyT<int>>
        {
            [[nodiscard]] size_t size() const
            {
                lock_guard<mutex> lock(_mutex);
                return _elements.size();
            }
        };

        auto factory = make_shared<TestKeyFactory>();
        factory->init();
        {
            auto key = factory->create(5);
            test(factory->size() == 1);

            // Creating the same value returns the interned element.
            test(factory->create(5) == key);
            test(factory->size() == 1);
        }
        test(factory->size() == 0); // the entry is erased when the element dies

        // The value can be interned again afterwards.
        auto key = factory->create(5);
        test(factory->size() == 1);
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
