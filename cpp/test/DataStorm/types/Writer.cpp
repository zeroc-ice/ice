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

namespace
{
    enum class color : unsigned char
    {
        blue,
        red,
    };

    // A key type with a custom encoding whose default value (0) encodes to zero bytes. The default Ice encoding
    // never produces an empty byte sequence, so only a custom Encoder can, and an any-key writer marshals such a
    // key inline like any other key.
    struct Marker
    {
        int value = 0;

        bool operator<(const Marker& other) const { return value < other.value; }
    };

    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    template<typename T, typename A, typename U> void testWriter(T topic, A add, U update)
    {
        topic.setWriterDefaultConfig(WriterConfig(-1, std::nullopt, ClearHistoryPolicy::Never));
        using WriterType = decltype(makeSingleKeyWriter(topic, typename decltype(add)::key_type()));
        map<typename decltype(topic)::KeyType, WriterType> writers;
        for (const auto& p : add)
        {
            writers.emplace(p.first, makeSingleKeyWriter(topic, p.first));
            writers.at(p.first).waitForReaders();
            writers.at(p.first).add(p.second);
        }
        for (const auto& p : update)
        {
            writers.at(p.first).update(p.second);
        }
        for (const auto& p : add)
        {
            writers.at(p.first).remove();
        }
        for (const auto& p : add)
        {
            writers.at(p.first).waitForNoReaders();
        }
    };
}

namespace DataStorm
{

    template<> struct Decoder<color>
    {
        static color decode(const Ice::CommunicatorPtr&, const vector<std::byte>& data)
        {
            return static_cast<color>(data[0]);
        }
    };

    template<> struct Encoder<color>
    {
        static vector<std::byte> encode(const Ice::CommunicatorPtr&, const color& value)
        {
            return {static_cast<std::byte>(value)};
        }
    };

    template<> struct Decoder<Marker>
    {
        static Marker decode(const Ice::CommunicatorPtr&, const vector<std::byte>& data)
        {
            return Marker{data.empty() ? 0 : static_cast<int>(data[0])};
        }
    };

    template<> struct Encoder<Marker>
    {
        static vector<std::byte> encode(const Ice::CommunicatorPtr&, const Marker& value)
        {
            // The default value encodes to zero bytes; any other value encodes to a single byte.
            if (value.value == 0)
            {
                return {};
            }
            return {static_cast<std::byte>(value.value)};
        }
    };

}

void ::Writer::run(int argc, char* argv[])
{
    Node node(argc, argv);

    cout << "testing string/string... " << flush;
    testWriter(
        Topic<string, string>(node, "stringstring"),
        map<string, string>{{"k1", "v1"}, {"k2", "v2"}},
        map<string, string>{{"k1", "u1"}, {"k2", "u2"}});
    cout << "ok" << endl;

    cout << "testing int/string... " << flush;
    testWriter(
        Topic<int, string>(node, "intstring"),
        map<int, string>{{1, "v1"}, {2, "v2"}},
        map<int, string>{{1, "u1"}, {2, "u2"}});
    cout << "ok" << endl;

    cout << "testing int/double... " << flush;
    testWriter(
        Topic<int, double>(node, "intdouble"),
        map<int, double>{{1, 2.0}, {2, 8.7}},
        map<int, double>{{1, 4.0}, {2, 7.8}});
    cout << "ok" << endl;

    cout << "testing string/struct... " << flush;
    testWriter(
        Topic<string, StructValue>(node, "stringstruct"),
        map<string, StructValue>{{"k1", {"firstName", "lastName", 10}}, {"k2", {"fn", "ln", 12}}},
        map<string, StructValue>{{"k1", {"firstName", "lastName", 18}}, {"k2", {"fn", "ln", 15}}});
    cout << "ok" << endl;

    cout << "testing struct/string... " << flush;
    testWriter(
        Topic<StructValue, string>(node, "structstring"),
        map<StructValue, string>{{{"firstName", "lastName", 10}, "v2"}, {{"fn", "ln", 12}, "v3"}},
        map<StructValue, string>{{{"firstName", "lastName", 10}, "v4"}, {{"fn", "ln", 12}, "v5"}});
    cout << "ok" << endl;

    cout << "testing string/class by ref... " << flush;
    testWriter(
        Topic<string, shared_ptr<Base>>(node, "stringclassbyref"),
        map<string, shared_ptr<Base>>{{"k1", make_shared<Base>("v1")}, {"k2", make_shared<Base>("v2")}},
        map<string, shared_ptr<Base>>{
            {"k1", make_shared<Extended>("v1", 10)},
            {"k2", make_shared<Extended>("v2", 10)}});
    cout << "ok" << endl;

    cout << "testing enum/string... " << flush;
    testWriter(
        Topic<color, string>(node, "enumstring"),
        map<color, string>{{color::blue, "v1"}, {color::red, "v2"}},
        map<color, string>{{color::blue, "u1"}, {color::red, "u2"}});
    cout << "ok" << endl;

    cout << "testing key that encodes to zero bytes... " << flush;
    {
        Topic<Marker, string> topic(node, "emptyencodedkey");
        topic.setWriterDefaultConfig(WriterConfig(-1, nullopt, ClearHistoryPolicy::Never));

        // An any-key writer marshals the key inline with each sample, rather than sending the key id.
        auto writer = makeAnyKeyWriter(topic);
        writer.waitForReaders();
        writer.add(Marker{0}, "v1");
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
