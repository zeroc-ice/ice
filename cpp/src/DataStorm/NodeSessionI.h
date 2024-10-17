//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_NODE_SESSIONI_H
#define DATASTORM_NODE_SESSIONI_H

#include "DataStorm/Contract.h"
#include "Ice/Ice.h"

namespace DataStormI
{

    class Instance;
    class TraceLevels;

    class NodeSessionI : public std::enable_shared_from_this<NodeSessionI>
    {
    public:
        NodeSessionI(std::shared_ptr<Instance>, std::optional<DataStormContract::NodePrx>, Ice::ConnectionPtr, bool);

        void init();
        void destroy();
        void addSession(std::optional<DataStormContract::SessionPrx>);

        DataStormContract::NodePrx getPublicNode() const {
            // always set after init
            assert(_publicNode);
            return *_publicNode; }
        std::optional<DataStormContract::LookupPrx> getLookup() const { return _lookup; }
        const Ice::ConnectionPtr& getConnection() const { return _connection; }
        template<typename T> std::optional<T> getSessionForwarder(std::optional<T> session) const
        {
            return Ice::uncheckedCast<T>(forwarder(session));
        }

    private:
        std::optional<DataStormContract::SessionPrx> forwarder(std::optional<DataStormContract::SessionPrx>) const;

        const std::shared_ptr<Instance> _instance;
        const std::shared_ptr<TraceLevels> _traceLevels;
        std::optional<DataStormContract::NodePrx> _node;
        const Ice::ConnectionPtr _connection;

        std::mutex _mutex;
        bool _destroyed;
        std::optional<DataStormContract::NodePrx> _publicNode;
        std::optional<DataStormContract::LookupPrx> _lookup;
        std::map<Ice::Identity, std::optional<DataStormContract::SessionPrx>> _sessions;
    };

}
#endif
