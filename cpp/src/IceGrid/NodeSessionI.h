// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_NODE_SESSION_H
#define ICEGRID_NODE_SESSION_H

#include "Internal.h"
#include <set>

namespace IceGrid
{
    class Database;
    class TraceLevels;

    class NodeSessionI final : public NodeSession, public std::enable_shared_from_this<NodeSessionI>
    {
    public:
        static std::shared_ptr<NodeSessionI> create(
            const std::shared_ptr<Database>&,
            NodePrx,
            const std::shared_ptr<InternalNodeInfo>&,
            std::chrono::seconds,
            const LoadInfo&);

        void keepAlive(LoadInfo, const Ice::Current&) final;
        void setReplicaObserver(std::optional<ReplicaObserverPrx>, const Ice::Current&) final;
        [[nodiscard]] int getTimeout(const Ice::Current&) const final;
        [[nodiscard]] std::optional<NodeObserverPrx> getObserver(const Ice::Current&) const final;
        void loadServersAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
            const final;
        [[nodiscard]] Ice::StringSeq getServers(const Ice::Current&) const final;
        void waitForApplicationUpdateAsync(
            std::string,
            int,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) const final;

        void destroy();
        void destroy(const Ice::Current&) final;

        [[nodiscard]] std::optional<std::chrono::steady_clock::time_point> timestamp() const noexcept;
        void shutdown();

        [[nodiscard]] const NodePrx& getNode() const;
        [[nodiscard]] const std::shared_ptr<InternalNodeInfo>& getInfo() const noexcept;
        [[nodiscard]] const LoadInfo& getLoadInfo() const;
        [[nodiscard]] NodeSessionPrx getProxy() const;

        [[nodiscard]] bool isDestroyed() const;

        [[nodiscard]] std::chrono::seconds timeout() const noexcept { return _timeout; }

    private:
        NodeSessionI(
            const std::shared_ptr<Database>&,
            NodePrx,
            const std::shared_ptr<InternalNodeInfo>&,
            std::chrono::seconds,
            NodeSessionPrx,
            const LoadInfo&);

        void destroyImpl(bool shutdown);

        const std::shared_ptr<Database> _database;
        const std::shared_ptr<TraceLevels> _traceLevels;
        const std::string _name;
        const NodePrx _node;
        const std::shared_ptr<InternalNodeInfo> _info;
        const std::chrono::seconds _timeout;
        NodeSessionPrx _proxy;
        std::optional<ReplicaObserverPrx> _replicaObserver;
        std::chrono::steady_clock::time_point _timestamp;
        LoadInfo _load;
        bool _destroy{false};

        mutable std::mutex _mutex;
    };

};

#endif
