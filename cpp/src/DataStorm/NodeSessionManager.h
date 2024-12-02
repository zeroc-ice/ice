//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_NODE_SESSION_MANAGER_H
#define DATASTORM_NODE_SESSION_MANAGER_H

#include "DataStorm/Config.h"
#include "DataStorm/Contract.h"
#include "Ice/Ice.h"

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

        std::shared_ptr<NodeSessionI> getSession(const Ice::Identity&) const;

        void forward(const Ice::ByteSeq&, const Ice::Current&) const;

    private:
        void connect(DataStormContract::LookupPrx, DataStormContract::NodePrx);

        void connected(DataStormContract::NodePrx, DataStormContract::LookupPrx);

        void disconnected(DataStormContract::NodePrx, DataStormContract::LookupPrx);
        void disconnected(DataStormContract::LookupPrx);

        void destroySession(std::optional<DataStormContract::NodePrx>);

        std::shared_ptr<Instance> getInstance() const
        {
            auto instance = _instance.lock();
            assert(instance);
            return instance;
        }

        std::weak_ptr<Instance> _instance;
        const std::shared_ptr<TraceLevels> _traceLevels;
        DataStormContract::NodePrx _nodePrx;
        const bool _forwardToMulticast;

        mutable std::mutex _mutex;

        int _retryCount;

        // A map containing the `NodeSessionI` servants for all nodes that have an active session with this node.
        // The map is indexed by the identity of the nodes.
        std::map<Ice::Identity, std::shared_ptr<NodeSessionI>> _sessions;

        // The `Lookup` proxy for the `ConnectTo` node, which is set when there is an active connection to the target
        // node. If the `DataStorm.Node.ConnectTo` property is configured, the session manager attempts to connect to
        // the specified node and sets this member once the connection is established.
        std::optional<DataStormContract::LookupPrx> _connectedTo;

        mutable Ice::ConnectionPtr _exclude;
        DataStormContract::LookupPrx _forwarder;
    };
}
#endif
