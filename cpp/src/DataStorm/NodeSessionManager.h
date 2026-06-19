// Copyright (c) ZeroC, Inc.

#ifndef DATASTORM_NODE_SESSION_MANAGER_H
#define DATASTORM_NODE_SESSION_MANAGER_H

#include "DataStorm/Contract.h"
#include "Ice/Ice.h"

#include <set>

namespace DataStormI
{
    class NodeSessionI;
    class CallbackExecutor;
    class Instance;
    class TraceLevels;
    class NodeI;

    class NodeSessionManager final : public std::enable_shared_from_this<NodeSessionManager>
    {
    public:
        NodeSessionManager(const std::shared_ptr<Instance>&, const std::shared_ptr<NodeI>&);

        void init();

        std::shared_ptr<NodeSessionI> createOrGet(DataStormContract::NodePrx, const Ice::ConnectionPtr&, bool);

        void
        announceTopicReader(const std::string&, DataStormContract::NodePrx, const Ice::ConnectionPtr& = nullptr) const;

        void
        announceTopicWriter(const std::string&, DataStormContract::NodePrx, const Ice::ConnectionPtr& = nullptr) const;

        void announceTopics(
            const Ice::StringSeq&,
            const Ice::StringSeq&,
            DataStormContract::NodePrx,
            const Ice::ConnectionPtr& = nullptr) const;

        [[nodiscard]] std::shared_ptr<NodeSessionI> getSession(const Ice::Identity&) const;

        void forward(const Ice::ByteSeq&, const Ice::Current&) const;

    private:
        // A topic announcement received from a peer over a node session, retained so it can be replayed when another
        // node session is (re)established later.
        struct Announcement
        {
            // The node proxy to include when replaying this announcement. For relayed nodes, this can be a
            // relay-created public proxy rather than the original proxy.
            DataStormContract::NodePrx node;

            // The connection of the node session the announcement was received on. Used to avoid echoing the
            // announcement back over that session and to drop the announcement when the connection closes.
            Ice::ConnectionPtr connection;

            std::set<std::string> readers;
            std::set<std::string> writers;
        };

        // Records a topic announcement so it can be replayed later. Announcements received over a connection that is
        // not a node session we track (e.g. the multicast lookup) are ignored. Must be called with `_mutex` held.
        void recordAnnouncement(
            const DataStormContract::NodePrx&,
            const Ice::StringSeq& readers,
            const Ice::StringSeq& writers,
            const Ice::ConnectionPtr&) const;

        // Replays every recorded announcement to the given lookup, skipping announcements received over the excluded
        // connection. Used to resynchronize topic announcements across a freshly (re)established node session. Must be
        // called with `_mutex` held.
        void announceKnownTopics(const DataStormContract::LookupPrx&, const Ice::ConnectionPtr& exclude) const;

        // Drops the announcements received over the given connection. Must be called with `_mutex` held.
        void removeAnnouncements(const Ice::ConnectionPtr&) const;

        // Returns true if this connection backs one of our node sessions - an inbound session we host in `_sessions`,
        // or the outbound session to our `_connectedTo` peer. Announcements can also arrive over the connectionless
        // multicast lookup, which has no node session and no connection we can track for eviction; those are
        // deliberately not cached. Must be called with `_mutex` held.
        [[nodiscard]] bool hasNodeSession(const Ice::ConnectionPtr&) const;

        void connect(const DataStormContract::LookupPrx&, const DataStormContract::NodePrx&);

        void connected(const DataStormContract::NodePrx&, const DataStormContract::LookupPrx&);

        void
        disconnected(const Ice::ConnectionPtr&, const DataStormContract::NodePrx&, const DataStormContract::LookupPrx&);

        void disconnected(const DataStormContract::LookupPrx&);

        void destroySession(const Ice::ConnectionPtr&, const DataStormContract::NodePrx&);

        std::weak_ptr<Instance> _instance;
        const std::shared_ptr<TraceLevels> _traceLevels;
        DataStormContract::NodePrx _nodePrx;
        const bool _forwardToMulticast;

        mutable std::mutex _mutex;

        int _retryCount{0};

        // A map containing the NodeSessionI servants for all nodes that have an active session with this node.
        // The map is indexed by the identity of the nodes.
        std::map<Ice::Identity, std::shared_ptr<NodeSessionI>> _sessions;

        // Topic announcements received from other nodes, indexed by the announced node identity.
        // We replay these announcements when a node session is established so topics received earlier
        // are not lost before they can be forwarded.
        //
        // Entries are removed when their source session closes or is replaced. Lookup announcements are additive
        // and idempotent; if a topic no longer exists, replaying its name is only a harmless discovery hint.
        mutable std::map<Ice::Identity, Announcement> _announcements;

        // The `Lookup` proxy for the `ConnectTo` node, which is set when there is an active connection to the target
        // node. If the `DataStorm.Node.ConnectTo` property is configured, the session manager attempts to connect to
        // the specified node and sets this member once the connection is established.
        std::optional<DataStormContract::LookupPrx> _connectedTo;

        // Stores the connection currently sending an announcement. This is used to exclude the node session
        // associated with the connection from the list of node sessions receiving the forwarded announcement.
        mutable Ice::ConnectionPtr _exclude;

        // A proxy to a collocated forwarder servant responsible for forwarding messages to the `Lookup`
        // objects of all active node sessions.
        DataStormContract::LookupPrx _forwarder;
    };
}
#endif
