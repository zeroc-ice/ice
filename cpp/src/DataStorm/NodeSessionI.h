//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_NODE_SESSIONI_H
#define DATASTORM_NODE_SESSIONI_H

#include "DataStorm/Contract.h"
#include "Ice/Ice.h"
#include "Instance.h"

namespace DataStormI
{
    class NodeSessionI final : public std::enable_shared_from_this<NodeSessionI>
    {
    public:
        NodeSessionI(std::shared_ptr<Instance>, DataStormContract::NodePrx, Ice::ConnectionPtr, bool);

        void init();
        void destroy();
        void addSession(DataStormContract::SessionPrx);

        DataStormContract::NodePrx getPublicNode() const
        {
            // always set after init
            assert(_publicNode);
            return *_publicNode;
        }

        std::optional<DataStormContract::LookupPrx> getLookup() const { return _lookup; }
        const Ice::ConnectionPtr& getConnection() const { return _connection; }

        // Helper method to create a forwarder proxy for a subscriber or publisher session proxy.
        template<typename Prx> Prx forwarder(Prx session) const
        {
            auto id = session->ice_getIdentity();
            auto proxy = _instance->getObjectAdapter()->createProxy<Prx>(
                {id.name + '-' + _node->ice_getIdentity().name, id.category + 'f'});
            return proxy->ice_oneway();
        }

    private:
        const std::shared_ptr<Instance> _instance;
        // A proxy to the target node, representing the node that created the session.
        DataStormContract::NodePrx _node;

        // The connection used to create the session.
        const Ice::ConnectionPtr _connection;

        std::mutex _mutex;
        // A proxy to the target node.
        //
        // - If the target node has a public endpoint or an adapter ID, this proxy is identical to the `_node` proxy.
        // - Otherwise, it is a proxy to a `NodeForwarder` object, which forwards calls to the target node over the
        //   incoming connection used to create the session.
        std::optional<DataStormContract::NodePrx> _publicNode;

        // A proxy for forwarding announcements to the target node when announce forwarding is enabled.
        // If announce forwarding is disabled, this will be nullopt.
        std::optional<DataStormContract::LookupPrx> _lookup;

        // A map containing all publisher and subscriber sessions established with the session's target node via a
        // node forwarder.
        std::map<Ice::Identity, DataStormContract::SessionPrx> _sessions;
    };
}
#endif
