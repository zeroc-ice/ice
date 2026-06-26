// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

using namespace DataStorm;
using namespace std;

namespace
{
    // Drives a hub node whose session to an endpoint-less peer ends up mid-retry, then tears the hub down and
    // reports whether the hub communicator was reclaimed (its weak_ptr expired).
    //
    // The hub has a public endpoint; the peer does not, so the hub reaches the peer over the connection the peer
    // establishes and cannot actively reconnect to it. When the peer goes away the hub's session follows the
    // "wait then remove" retry path: it schedules a timer that removes the session if the peer doesn't return.
    //
    // With a short retry delay the timer fires remove() -> destroyImpl before the hub is destroyed (exercising the
    // destroyImpl teardown path). With a long retry delay the session is still mid-retry when the hub is destroyed
    // (exercising the NodeI::destroy teardown path). In both cases the pending task captures a strong reference to
    // the session, so unless that reference is cleared the session -- and, through its strong reference to the
    // instance, the hub communicator -- leaks.
    bool hubReclaimed(const Ice::PropertiesPtr& base, const string& endpoint, bool letRemoveTimerFire)
    {
        weak_ptr<Ice::Communicator> weakHub;
        {
            // Set every DataStorm.Node.* property the test relies on explicitly, so the test harness's default
            // node properties (which it injects when a case specifies none) can't bleed in via the shared base.
            Ice::InitializationData hubInit;
            hubInit.properties = base->clone();
            hubInit.properties->setProperty("DataStorm.Node.Server.Enabled", "1");
            hubInit.properties->setProperty("DataStorm.Node.Server.Endpoints", endpoint);
            hubInit.properties->setProperty("DataStorm.Node.ConnectTo", "");
            hubInit.properties->setProperty("DataStorm.Node.Multicast.Enabled", "0");
            hubInit.properties->setProperty("DataStorm.Node.RetryMultiplier", "1");
            hubInit.properties->setProperty("DataStorm.Node.RetryCount", "1");
            // Retry delay is RetryDelay * RetryMultiplier^count * 2. ~2ms lets the remove timer fire; a long delay
            // keeps the session mid-retry until the hub is destroyed.
            hubInit.properties->setProperty("DataStorm.Node.RetryDelay", letRemoveTimerFire ? "1" : "60000");
            hubInit.properties->setProperty("Ice.Warn.Connections", "0");

            Ice::CommunicatorPtr hubCommunicator = Ice::initialize(hubInit);
            weakHub = hubCommunicator;

            {
                Node hub{hubCommunicator};
                Topic<string, int> topic{hub, "topic"};
                auto reader = makeSingleKeyReader(topic, "key");

                {
                    Ice::InitializationData peerInit;
                    peerInit.properties = base->clone();
                    peerInit.properties->setProperty("DataStorm.Node.Server.Enabled", "0");
                    peerInit.properties->setProperty("DataStorm.Node.Server.Endpoints", "");
                    peerInit.properties->setProperty("DataStorm.Node.ConnectTo", endpoint);
                    peerInit.properties->setProperty("DataStorm.Node.Multicast.Enabled", "0");
                    peerInit.properties->setProperty("Ice.Warn.Connections", "0");

                    Ice::CommunicatorPtr peerCommunicator = Ice::initialize(peerInit);
                    try
                    {
                        Node peer{peerCommunicator};
                        Topic<string, int> peerTopic{peer, "topic"};
                        auto writer = makeSingleKeyWriter(peerTopic, "key");

                        // Establish the session and confirm data flows through it.
                        writer.waitForReaders();
                        writer.update(42);
                        test(reader.getNextUnread().getValue() == 42);
                    }
                    catch (...)
                    {
                        peerCommunicator->destroy();
                        throw;
                    }
                    peerCommunicator->destroy();
                }

                // The endpoint-less peer is gone. Wait for the hub to see the writer disappear, then leave time for
                // the session to enter the wait-then-remove retry path (and, with the short delay, for the remove
                // timer to fire). The long-delay case is still mid-retry when the hub is destroyed below.
                reader.waitForNoWriters();
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            hubCommunicator->destroy();
        }

        // The session must be reclaimed once we drop our last reference to the communicator above. A bounded poll
        // tolerates any asynchronous teardown latency; a leaked session never lets the weak_ptr expire.
        for (int i = 0; i < 200 && !weakHub.expired(); ++i)
        {
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        return weakHub.expired();
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
    const string endpoint = getTestEndpoint(base, 0);

    cout << "testing that a removed mid-retry session does not leak... " << flush;
    test(hubReclaimed(base, endpoint, true));
    cout << "ok" << endl;

    cout << "testing that destroying a node mid-retry does not leak the session... " << flush;
    test(hubReclaimed(base, endpoint, false));
    cout << "ok" << endl;
}

DEFINE_TEST(::Writer)
