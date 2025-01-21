// Copyright (c) ZeroC, Inc.
#if defined(_MSC_VER)
#    pragma warning(4 : 4244)
#endif

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

    Topic<string, StockPtr> topic(node, "topic");

    WriterConfig config;
    config.sampleCount = -1;
    config.clearHistory = ClearHistoryPolicy::Never;
    topic.setWriterDefaultConfig(config);

    topic.setUpdater<float>("price", [](StockPtr& stock, float price) { stock->price = price; });

    cout << "testing partial update... " << flush;
    {
        auto writer = makeSingleKeyWriter(topic, "AAPL");
        writer.waitForReaders();
        writer.add(make_shared<Stock>(12.0f, 13.0f, 14.0f));
        writer.partialUpdate<float>("price")(15.0f);
        writer.partialUpdate<float>("price")(18);
        writer.waitForNoReaders();
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
