// Copyright (c) ZeroC, Inc.

#include "Test.h"
#include "TestHelper.h"

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;
using namespace DataStormContract;

namespace
{
    // Stands in for the peer's session servant. What the node under test sends over an established session is
    // irrelevant here, so accepting every operation avoids implementing the whole Session interface. The
    // operations are counted because they tell the test what the node decided: a node announces its topics to a
    // session it has just connected, and disconnects one it has refused to connect.
    class SessionStub final : public Ice::Blobject
    {
    public:
        bool ice_invoke(Ice::ByteSeq inParams, Ice::ByteSeq&, const Ice::Current& current) final
        {
            {
                lock_guard<mutex> lock(_mutex);
                ++_counts[current.operation];
                if (current.operation == "disconnected")
                {
                    // Decode the handshake the notification names: the point of the notification is that it is
                    // scoped, so a test that only counted the operation would pass on a notification naming the
                    // wrong handshake - which is the harmful case, since it tears down a session that is fine.
                    Ice::InputStream in{current.adapter->getCommunicator(), inParams};
                    in.startEncapsulation();
                    int64_t handshakeId{0};
                    in.read(handshakeId);
                    in.endEncapsulation();
                    _disconnected.push_back(handshakeId);
                }
            }
            _condition.notify_all();
            return true;
        }

        [[nodiscard]] int count(const string& operation)
        {
            lock_guard<mutex> lock(_mutex);
            return _counts[operation];
        }

        /// Waits for @p operation to be dispatched more than @p count times, and reports whether it was.
        [[nodiscard]] bool waitFor(const string& operation, int count, chrono::milliseconds timeout)
        {
            unique_lock<mutex> lock(_mutex);
            return _condition.wait_for(lock, timeout, [this, &operation, count] { return _counts[operation] > count; });
        }

        /// Reports whether a disconnect notification naming @p handshakeId has arrived.
        [[nodiscard]] bool sawDisconnect(int64_t handshakeId)
        {
            lock_guard<mutex> lock(_mutex);
            return find(_disconnected.begin(), _disconnected.end(), handshakeId) != _disconnected.end();
        }

        /// Waits for a disconnect notification naming @p handshakeId, and reports whether one arrived.
        [[nodiscard]] bool waitForDisconnect(int64_t handshakeId, chrono::milliseconds timeout)
        {
            unique_lock<mutex> lock(_mutex);
            return _condition.wait_for(
                lock,
                timeout,
                [this, handshakeId]
                { return find(_disconnected.begin(), _disconnected.end(), handshakeId) != _disconnected.end(); });
        }

    private:
        mutex _mutex;
        condition_variable _condition;
        map<string, int> _counts;
        vector<int64_t> _disconnected;
    };

    // A peer node that plays the subscriber against the node under test, and holds each confirmCreateSession
    // response until the test releases it. Holding the response is what lets the test place a session creation
    // handshake's completion after the events that supersede it.
    class PeerNode final : public AsyncNode
    {
    public:
        struct Confirmation
        {
            optional<PublisherSessionPrx> session;
            Ice::ConnectionPtr connection;
            int64_t handshakeId;
            function<void()> response;
            function<void(exception_ptr)> exception;
        };

        void initiateCreateSessionAsync(
            optional<NodePrx> publisher,
            function<void()> response,
            function<void(exception_ptr)>,
            const Ice::Current&) final
        {
            {
                lock_guard<mutex> lock(_mutex);
                _publisherNode = std::move(publisher);
                ++_initiateCount;
            }
            _condition.notify_all();
            response();
        }

        // The node under test is the publisher; it never asks this peer to create a session.
        void createSessionAsync(
            optional<NodePrx>,
            optional<SubscriberSessionPrx>,
            bool,
            int64_t,
            function<void()> response,
            function<void(exception_ptr)>,
            const Ice::Current&) final
        {
            response();
        }

        void confirmCreateSessionAsync(
            optional<NodePrx>,
            optional<PublisherSessionPrx> session,
            int64_t handshakeId,
            function<void()> response,
            function<void(exception_ptr)> exception,
            const Ice::Current& current) final
        {
            {
                lock_guard<mutex> lock(_mutex);
                // The node addresses this session over the connection the createSession request arrived on, so the
                // test has to answer it over that same connection: a disconnect notification that reaches the node
                // over any other connection is ignored as belonging to a superseded incarnation.
                _confirmations.push_back(Confirmation{
                    std::move(session),
                    current.con,
                    handshakeId,
                    std::move(response),
                    std::move(exception)});
            }
            _condition.notify_all();
        }

        [[nodiscard]] NodePrx waitForPublisherNode()
        {
            unique_lock<mutex> lock(_mutex);
            _condition.wait(lock, [this] { return _publisherNode.has_value(); });
            return *_publisherNode;
        }

        /// The number of times the node asked this peer to initiate session creation. The node does that when it
        /// schedules a reconnection, so it reports whether a failure consumed the session's retry budget.
        [[nodiscard]] int initiateCount()
        {
            lock_guard<mutex> lock(_mutex);
            return _initiateCount;
        }

        /// Waits for the node to ask this peer to initiate session creation more than @p count times.
        [[nodiscard]] bool waitForInitiate(int count, chrono::milliseconds timeout)
        {
            unique_lock<mutex> lock(_mutex);
            return _condition.wait_for(lock, timeout, [this, count] { return _initiateCount > count; });
        }

        [[nodiscard]] Confirmation waitForConfirmation()
        {
            unique_lock<mutex> lock(_mutex);
            _condition.wait(lock, [this] { return !_confirmations.empty(); });
            Confirmation confirmation = std::move(_confirmations.front());
            _confirmations.pop_front();
            return confirmation;
        }

    private:
        mutex _mutex;
        condition_variable _condition;
        optional<NodePrx> _publisherNode;
        int _initiateCount{0};
        deque<Confirmation> _confirmations;
    };
}

class Client : public Test::TestHelper
{
public:
    Client() : Test::TestHelper(false) {}

    void run(int, char**) override;
};

void ::Client::run(int argc, char* argv[])
{
    Ice::CommunicatorHolder holder{initialize(argc, argv)};
    auto communicator = holder.communicator();
    auto adapter = communicator->createObjectAdapterWithEndpoints("peer", getTestEndpoint(12));
    auto peer = make_shared<PeerNode>();
    auto peerNode = adapter->add<NodePrx>(peer, Ice::Identity{.name = "peer-app", .category = ""});
    auto session = make_shared<SessionStub>();
    adapter->addDefaultServant(session, "s");
    adapter->activate();
    auto peerSession = adapter->createProxy<SubscriberSessionPrx>(Ice::Identity{.name = "1", .category = "s"});

    cout << "testing that a superseded session confirmation does not connect the session... " << flush;
    {
        // Announcing a reader for the topic the node writes makes the node drive session creation with this peer.
        LookupPrx lookup{communicator, "DataStorm/Lookup2:" + getTestEndpoint(11)};

        // The node under test may still be starting up.
        while (true)
        {
            try
            {
                lookup->announceTopicReader("string", peerNode);
                break;
            }
            catch (const Ice::LocalException&)
            {
                this_thread::sleep_for(chrono::milliseconds(50));
            }
        }
        auto publisher = peer->waitForPublisherNode();

        // The node fixes the session proxies to the connection createSession arrives on, so this peer has to
        // dispatch the node's callbacks on that connection.
        publisher->ice_getConnection()->setAdapter(adapter);

        // First handshake: hold its confirmation, so that it completes only after later events superseded it.
        publisher->createSession(peerNode, peerSession, false, 1);
        auto first = peer->waitForConfirmation();
        test(first.handshakeId == 1);

        // Second handshake, answered right away: the node connects the session under this one.
        publisher->createSession(peerNode, peerSession, false, 2);
        auto second = peer->waitForConfirmation();
        test(second.handshakeId == 2);
        second.response();

        // Drop the session the node just connected, naming the handshake it is connected under.
        second.session->ice_fixed(second.connection)->ice_twoway()->disconnected(2);

        // Release the superseded confirmation. The node must not commit it: it announces its topics to a session
        // it has just connected, so an announcement here would mean it acted on a handshake this peer had already
        // replaced.
        const int announced = session->count("announceTopics");
        first.response();
        test(!session->waitFor("announceTopics", announced, chrono::seconds(1)));

        // Having refused the confirmation, the node must tell this peer to drop that handshake: the peer connects
        // its session before answering a confirmation, so it is left connected to a session the node is not using
        // and would wait forever for samples that never arrive.
        // Scoped to the refused handshake, and to that one only: a notification naming the handshake this peer is
        // working on would tear down a session that is fine, which is the failure the scoping exists to prevent.
        test(session->waitForDisconnect(1, chrono::seconds(10)));
        test(!session->sawDisconnect(2));

        // The node is expected to accept a fresh handshake. While the bug is present it answers AlreadyConnected
        // forever, because the superseded confirmation connected it to a session this peer no longer has.
        bool accepted = false;
        for (int i = 0; i < 30 && !accepted; ++i)
        {
            try
            {
                publisher->createSession(peerNode, peerSession, false, 3);
                accepted = true;
            }
            catch (const SessionCreationException& ex)
            {
                test(ex.error == SessionCreationError::AlreadyConnected);
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
        test(accepted);

        auto third = peer->waitForConfirmation();
        test(third.handshakeId == 3);

        // The node commits the handshake when it processes this reply, and announces its topics to this peer's
        // session once it has. The checks below are about the committed handshake, so wait for that announcement
        // rather than assuming the reply was processed by the time the next request arrives.
        const int connecting = session->count("announceTopics");
        third.response();
        test(session->waitFor("announceTopics", connecting, chrono::seconds(10)));

        // A repeat of the handshake the node is connected under is a duplicate and is rejected as such.
        try
        {
            publisher->createSession(peerNode, peerSession, false, 3);
            test(false);
        }
        catch (const SessionCreationException& ex)
        {
            test(ex.error == SessionCreationError::AlreadyConnected);
        }

        // A different handshake is proof that this peer no longer has the session the node is connected under, so
        // the node gives up that session and takes the new handshake instead of stonewalling the peer.
        publisher->createSession(peerNode, peerSession, false, 4);
        auto fourth = peer->waitForConfirmation();
        test(fourth.handshakeId == 4);

        // Wait for the commit, as after handshake 3: the reply is processed asynchronously, and the next scenario
        // starts from the node being connected under this handshake.
        const int connected4 = session->count("announceTopics");
        fourth.response();
        test(session->waitFor("announceTopics", connected4, chrono::seconds(10)));
    }
    cout << "ok" << endl;

    cout << "testing that a superseded confirmation does not consume the retry budget... " << flush;
    {
        // The node is connected under handshake 4. Start handshake 5 and refuse its confirmation the way a peer
        // does when it has already moved to a newer handshake of its own.
        auto publisher = peer->waitForPublisherNode();
        const int initiated = peer->initiateCount();

        publisher->createSession(peerNode, peerSession, false, 5);
        auto fifth = peer->waitForConfirmation();
        test(fifth.handshakeId == 5);
        fifth.exception(make_exception_ptr(SessionCreationException{SessionCreationError::Superseded}));

        // This peer allocates the handshake identifiers, so the node has to take its word for it and leave the
        // session to the handshake the peer says it started. Spending a retry here would prompt a reconnection -
        // which reaches this peer as a request to initiate session creation - and each such round would refuse
        // its predecessor the same way, exhausting the budget of a session that is recovering normally.
        test(!peer->waitForInitiate(initiated, chrono::seconds(2)));

        // Leaving the session to the peer does not mean forgetting it: the peer's word covers the handshake it
        // allocated, not its delivery, so the node parks the session and removes it if the peer never does
        // re-establish it. Once that happens a fresh handshake is served by a new session servant.
        const auto parked = fifth.session->ice_getIdentity();
        bool replaced = false;
        for (int i = 0; i < 100 && !replaced; ++i)
        {
            publisher->createSession(peerNode, peerSession, false, 6 + i);
            auto next = peer->waitForConfirmation();
            replaced = next.session->ice_getIdentity() != parked;
            next.exception(make_exception_ptr(SessionCreationException{SessionCreationError::Superseded}));
            if (!replaced)
            {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
        test(replaced);
    }
    cout << "ok" << endl;

    cout << "testing that re-establishing a parked session cancels its removal... " << flush;
    {
        auto publisher = peer->waitForPublisherNode();

        // Park the session again, then re-establish it before the removal timer expires.
        publisher->createSession(peerNode, peerSession, false, 200);
        auto parked = peer->waitForConfirmation();
        test(parked.handshakeId == 200);
        const auto servant = parked.session->ice_getIdentity();
        parked.exception(make_exception_ptr(SessionCreationException{SessionCreationError::Superseded}));

        publisher->createSession(peerNode, peerSession, false, 201);
        auto revived = peer->waitForConfirmation();
        test(revived.handshakeId == 201);
        test(revived.session->ice_getIdentity() == servant);
        const int connected201 = session->count("announceTopics");
        revived.response();
        test(session->waitFor("announceTopics", connected201, chrono::seconds(10)));

        // Well past the park timeout the session must still be the one that was parked: connecting cancels the
        // removal, rather than the timer firing later and dropping a session the peer is using.
        this_thread::sleep_for(chrono::seconds(3));
        publisher->createSession(peerNode, peerSession, false, 202);
        auto after = peer->waitForConfirmation();
        test(after.session->ice_getIdentity() == servant);
        after.response();
    }
    cout << "ok" << endl;

    cout << "testing that stale requests and notifications are ignored... " << flush;
    {
        auto publisher = peer->waitForPublisherNode();

        // Start handshake 400 and hold its confirmation, leaving the session disconnected with 400 pending.
        publisher->createSession(peerNode, peerSession, false, 400);
        auto pending = peer->waitForConfirmation();
        test(pending.handshakeId == 400);

        // A request for a handshake older than the pending one is refused rather than adopted. Requests take
        // different routes and arrive in any order, so adopting an older one would leave the node waiting on a
        // handshake this peer has already moved past, and refuse the newer one's confirmation when it completes.
        try
        {
            publisher->createSession(peerNode, peerSession, false, 399);
            test(false);
        }
        catch (const SessionCreationException& ex)
        {
            test(ex.error == SessionCreationError::Superseded);
        }

        // A disconnect naming that older handshake, while the session is disconnected with a newer one pending,
        // reports the loss of a route the session no longer uses. Treating it as a fresh failure would spend a
        // retry, which reaches this peer as a request to initiate session creation.
        auto session400 = pending.session->ice_fixed(pending.connection)->ice_twoway();
        const int initiated = peer->initiateCount();
        session400->disconnected(399);
        test(!peer->waitForInitiate(initiated, chrono::seconds(2)));

        const int connected400 = session->count("announceTopics");
        pending.response();
        test(session->waitFor("announceTopics", connected400, chrono::seconds(10)));

        // The same stale notification, now that the session is connected under the newer handshake, leaves it
        // alone: the node still treats handshake 400 as the one it is connected under.
        session400->disconnected(399);
        try
        {
            publisher->createSession(peerNode, peerSession, false, 400);
            test(false);
        }
        catch (const SessionCreationException& ex)
        {
            test(ex.error == SessionCreationError::AlreadyConnected);
        }
    }
    cout << "ok" << endl;
}

DEFINE_TEST(::Client)
