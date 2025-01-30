// Copyright (c) ZeroC, Inc.

#if defined(_WIN32)
#    pragma warning(disable : 4503) // decorated name length exceeded, name was truncated
#endif

#include "DataStorm/DataStorm.h"
#include "Test.h"
#include "TestHelper.h"

using namespace DataStorm;
using namespace std;
using namespace Test;

class Reader : public Test::TestHelper
{
public:
    Reader() : Test::TestHelper(false) {}

    void run(int, char**) override;
};

namespace
{

    enum class color : unsigned char
    {
        blue,
        red,
    };

    template<typename T> bool compare(const T& v1, const T& v2) { return v1 == v2; }

    template<typename T> bool compare(const shared_ptr<T>& v1, const shared_ptr<T>& v2)
    {
        return v1->ice_tuple() == v2->ice_tuple();
    }

    template<typename T, typename A, typename U> void testReader(T topic, const A& add, const U& update)
    {
        topic.setReaderDefaultConfig(ReaderConfig(-1, std::nullopt, ClearHistoryPolicy::Never));
        map<typename decltype(topic)::KeyType, typename decltype(topic)::ReaderType> readers;
        for (const auto& p : add)
        {
            readers.emplace(p.first, makeSingleKeyReader(topic, p.first));
            auto s = readers.at(p.first).getNextUnread();
            test(s.getEvent() == SampleEvent::Add && compare(s.getValue(), p.second));
        }
        for (const auto& p : update)
        {
            auto s = readers.at(p.first).getNextUnread();
            test(s.getEvent() == SampleEvent::Update && compare(s.getValue(), p.second));
        }
        for (const auto& p : add)
        {
            auto s = readers.at(p.first).getNextUnread();
            test(s.getEvent() == SampleEvent::Remove);
        }
    };

} // namespace

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

}

void ::Reader::run(int argc, char* argv[])
{
    Node node(argc, argv);

    testReader(
        Topic<string, string>(node, "stringstring"),
        map<string, string>{{"k1", "v1"}, {"k2", "v2"}},
        map<string, string>{{"k1", "u1"}, {"k2", "u2"}});
    testReader(
        Topic<int, string>(node, "intstring"),
        map<int, string>{{1, "v1"}, {2, "v2"}},
        map<int, string>{{1, "u1"}, {2, "u2"}});
    testReader(
        Topic<int, double>(node, "intdouble"),
        map<int, double>{{1, 2.0}, {2, 8.7}},
        map<int, double>{{1, 4.0}, {2, 7.8}});
    testReader(
        Topic<string, StructValue>(node, "stringstruct"),
        map<string, StructValue>{{"k1", {"firstName", "lastName", 10}}, {"k2", {"fn", "ln", 12}}},
        map<string, StructValue>{{"k1", {"firstName", "lastName", 18}}, {"k2", {"fn", "ln", 15}}});
    testReader(
        Topic<StructValue, string>(node, "structstring"),
        map<StructValue, string>{{{"firstName", "lastName", 10}, "v2"}, {{"fn", "ln", 12}, "v3"}},
        map<StructValue, string>{{{"firstName", "lastName", 10}, "v4"}, {{"fn", "ln", 12}, "v5"}});

    testReader(
        Topic<string, shared_ptr<Base>>(node, "stringclassbyref"),
        map<string, shared_ptr<Base>>{{"k1", make_shared<Base>("v1")}, {"k2", make_shared<Base>("v2")}},
        map<string, shared_ptr<Base>>{
            {"k1", make_shared<Extended>("v1", 10)},
            {"k2", make_shared<Extended>("v2", 10)}});
    testReader(
        Topic<color, string>(node, "enumstring"),
        map<color, string>{{color::blue, "v1"}, {color::red, "v2"}},
        map<color, string>{{color::blue, "u1"}, {color::red, "u2"}});
}

DEFINE_TEST(::Reader)
