// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "TestHelper.h"
#include "TestKey.h"

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
    config.sampleCount = -1;
    config.clearHistory = ClearHistoryPolicy::Never;

    {
        // An any-key reader matches every key of the writer, so the writer answers with one element spec per key and
        // each spec carries its key inline. Decoding them here rejects "poison", and the spec for "good" must still
        // attach.
        Topic<TestKey, string> topic(node, "announcedKeys");
        auto reader = makeAnyKeyReader(topic, "", config);

        auto sample = reader.getNextUnread();
        test(sample.getKey() == TestKey{"good"});
        test(sample.getValue() == "announced");
    }

    {
        // Both readers travel to the any-key writer in the same request, each carrying its key inline. The writer
        // cannot decode "poison", and the reader on "good" must attach regardless.
        Topic<TestKey, string> topic(node, "anyKey");
        auto poisonReader = makeSingleKeyReader(topic, TestKey{"poison"}, "", config);
        auto reader = makeSingleKeyReader(topic, TestKey{"good"}, "", config);

        auto sample = reader.getNextUnread();
        test(sample.getKey() == TestKey{"good"});
        test(sample.getValue() == "spec");
        test(!poisonReader.hasUnread());
    }

    {
        // The key-filtered reader's criteria is a key the writer cannot decode. Matching that filter against the
        // writer's keys must not cost the plain reader announced in the same request its attachment.
        Topic<TestKey, string> topic(node, "keyFilter");
        // The reader resolves the filter locally from the criteria object; only the writer decodes it off the wire.
        topic.setKeyFilter<TestKey>(
            "named",
            [](const TestKey& criteria) { return [criteria](const TestKey& key) { return key == criteria; }; });

        auto filteredReader = makeFilteredKeyReader(topic, Filter<TestKey>("named", TestKey{"poison"}), "", config);
        auto reader = makeSingleKeyReader(topic, TestKey{"good"}, "", config);

        auto sample = reader.getNextUnread();
        test(sample.getValue() == "keyFiltered");
        test(!filteredReader.hasUnread());
    }

    {
        // The filtered reader's criteria is a key the writer cannot decode. The writer has to leave that reader
        // unattached without dropping the unfiltered reader attached in the same request.
        Topic<TestKey, string> topic(node, "sampleFilter");
        auto filteredReader =
            makeSingleKeyReader(topic, TestKey{"good"}, Filter<TestKey>("named", TestKey{"poison"}), "", config);
        auto reader = makeSingleKeyReader(topic, TestKey{"good"}, "", config);

        auto sample = reader.getNextUnread();
        test(sample.getValue() == "filtered");
        test(!filteredReader.hasUnread());
    }
}

DEFINE_TEST(::Reader)
