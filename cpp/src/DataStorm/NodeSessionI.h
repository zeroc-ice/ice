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
        DataStormContract::NodePrx _node;
        const Ice::ConnectionPtr _connection;

        std::mutex _mutex;
        bool _destroyed;
        std::optional<DataStormContract::NodePrx> _publicNode;

        // A proxy for forwarding announcements to the target node when announce forwarding is enabled.
        // If announce forwarding is disabled, this will be nullopt.
        std::optional<DataStormContract::LookupPrx> _lookup;

        // A map containing all the publisher and subscriber sessions established between two nodes.
        std::map<Ice::Identity, std::optional<DataStormContract::SessionPrx>> _sessions;
    };
}
#endif
