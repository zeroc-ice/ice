// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

#include <atomic>
#include <chrono>
#include <thread>

using namespace DataStorm;
using namespace std;

namespace
{
    // H3: a bounded read. DataStorm's getNextUnread blocks forever, so a lost-sample regression turns into a silent
    // CI hang. This polls hasUnread with a deadline and, on expiry, fails with context instead of hanging.
    template<typename ReaderT>
    int boundedNextUnread(ReaderT& reader, int expected, chrono::seconds timeout = chrono::seconds(30))
    {
        auto deadline = chrono::steady_clock::now() + timeout;
        while (!reader.hasUnread())
        {
            if (chrono::steady_clock::now() >= deadline)
            {
                cerr << "timed out waiting for a sample (expected value " << expected
                     << "): a sample was lost across a relay-hop drop" << endl;
                test(false);
            }
            this_thread::sleep_for(chrono::milliseconds(5));
        }
        return reader.getNextUnread().getValue();
    }

    Ice::CommunicatorPtr
    makeNode(const Ice::PropertiesPtr& base, const string& name, const string& endpoint, const string& connectTo)
    {
        Ice::InitializationData init;
        init.properties = base->clone();
        init.properties->setProperty("DataStorm.Node.Name", name);
        init.properties->setProperty("Ice.ProgramName", name);
        init.properties->setProperty("DataStorm.Node.Multicast.Enabled", "0");
        init.properties->setProperty("DataStorm.Node.Server.Enabled", endpoint.empty() ? "0" : "1");
        init.properties->setProperty("DataStorm.Node.Server.Endpoints", endpoint);
        init.properties->setProperty("DataStorm.Node.ConnectTo", connectTo);
        init.properties->setProperty("Ice.Warn.Connections", "0");
        return Ice::initialize(init);
    }
}

class Writer : public Test::TestHelper
{
public:
    Writer() : Test::TestHelper(false) {}

    void run(int, char**) override;
};

void ::Writer::run(int argc, char* argv[])
{
    Ice::PropertiesPtr base = createTestProperties(argc, argv);
    const string ep1 = getTestEndpoint(base, 0);
    const string ep2 = getTestEndpoint(base, 1);

    cout << "testing 2-hop forwarding across relay-to-relay drops while the writer is active... " << flush;

    // Chain: writer -> relay1 -> relay2 -> reader. The writer and relay2 both dial relay1 (ep1); the reader dials
    // relay2 (ep2). The hop we repeatedly drop is relay2's connection to relay1 -- a non-app, relay-to-relay
    // connection that no existing test closes.
    Ice::CommunicatorPtr relay1Comm = makeNode(base, "relay1", ep1, "");
    Ice::CommunicatorPtr relay2Comm = makeNode(base, "relay2", ep2, ep1);
    Ice::CommunicatorPtr writerComm = makeNode(base, "writer", "", ep1);
    Ice::CommunicatorPtr readerComm = makeNode(base, "reader", "", ep2);

    const int sampleCount = 500;
    try
    {
        Node relay1{relay1Comm};
        Node relay2{relay2Comm};
        Node writerNode{writerComm};
        Node readerNode{readerComm};

        WriterConfig writerConfig;
        writerConfig.clearHistory = ClearHistoryPolicy::Never;
        ReaderConfig readerConfig;
        readerConfig.clearHistory = ClearHistoryPolicy::Never;

        Topic<string, int> writerTopic{writerNode, "topic"};
        Topic<string, int> readerTopic{readerNode, "topic"};
        auto writer = makeSingleKeyWriter(writerTopic, "key", "", writerConfig);
        auto reader = makeSingleKeyReader(readerTopic, "key", "", readerConfig);

        // The reader is attached through both relays once this returns.
        writer.waitForReaders();

        // Publish a known sequence at a steady pace on a background thread, so the writer is still forwarding samples
        // through the relays while the main thread drops the relay-to-relay hop.
        atomic<bool> writerFailed{false};
        thread writerThread(
            [&]
            {
                try
                {
                    for (int i = 0; i < sampleCount; ++i)
                    {
                        writer.update(i);
                        this_thread::sleep_for(chrono::milliseconds(2));
                    }
                }
                catch (...)
                {
                    writerFailed = true;
                }
            });

        // Read the whole sequence, dropping the relay-to-relay hop every 50 samples while the writer keeps
        // publishing. Each sample must arrive exactly once and in order: a sample lost or duplicated by a relay-hop
        // drop while it was mid-forward would surface here as a gap (bounded-read timeout), a duplicate, or a rewind.
        for (int i = 0; i < sampleCount; ++i)
        {
            int v = boundedNextUnread(reader, i);
            if (v != i)
            {
                cerr << "duplicate, gap, or rewound sample across a relay-hop drop: " << v << " expected " << i << endl;
                test(false);
            }

            if (i > 0 && (i % 50) == 0)
            {
                // Ice caches one connection per endpoint, so a proxy to relay1's Lookup endpoint returns the
                // connection relay2 currently uses for the relay session; closing it drops the middle hop without
                // touching the writer or reader connections. relay2 reconnects immediately, so we re-fetch each time.
                if (auto hop = relay2Comm->stringToProxy("DataStorm/Lookup:" + ep1)->ice_getConnection())
                {
                    hop->close().get();
                }
            }
        }

        writerThread.join();
        test(!writerFailed);
    }
    catch (...)
    {
        readerComm->destroy();
        writerComm->destroy();
        relay2Comm->destroy();
        relay1Comm->destroy();
        throw;
    }
    readerComm->destroy();
    writerComm->destroy();
    relay2Comm->destroy();
    relay1Comm->destroy();

    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
