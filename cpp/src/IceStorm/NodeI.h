// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_NODE_I_H
#define ICESTORM_NODE_I_H

#include "../Ice/Timer.h"
#include "Election.h"
#include "Ice/Ice.h"
#include "Instance.h"
#include "Replica.h"

#include <condition_variable>
#include <set>

namespace IceStormElection
{
    class Observers;

    class NodeI final : public Node, public std::enable_shared_from_this<NodeI>
    {
    public:
        NodeI(
            const std::shared_ptr<IceStorm::Instance>&,
            std::shared_ptr<Replica>,
            Ice::ObjectPrx,
            int,
            const std::map<int, NodePrx>&);

        void start();

        void check();
        void timeout();
        void merge(const std::set<int>&);
        void mergeContinue();
        void invitation(int, std::string, const Ice::Current&) final;
        void ready(int, std::string, std::optional<Ice::ObjectPrx>, int, std::int64_t, const Ice::Current&) final;
        void
        accept(int, std::string, Ice::IntSeq, std::optional<Ice::ObjectPrx>, LogUpdate, int, const Ice::Current&) final;
        [[nodiscard]] bool areYouCoordinator(const Ice::Current&) const final;
        [[nodiscard]] bool areYouThere(std::string, int, const Ice::Current&) const final;
        [[nodiscard]] std::optional<Ice::ObjectPrx> sync(const Ice::Current&) const final;
        [[nodiscard]] NodeInfoSeq nodes(const Ice::Current&) const final;
        [[nodiscard]] QueryInfo query(const Ice::Current&) const final;
        void recovery(std::int64_t = -1);

        void destroy();

        // Notify the node that we're about to start an update.
        void checkObserverInit(std::int64_t);
        std::optional<Ice::ObjectPrx> startUpdate(std::int64_t&, const char*, int);
        std::optional<Ice::ObjectPrx> startCachedRead(std::int64_t&, const char*, int);
        void startObserverUpdate(std::int64_t, const char*, int);
        bool updateMaster(const char*, int);

        // The node has completed the update.
        void finishUpdate();

    private:
        void setState(NodeState);

        const IceInternal::TimerPtr _timer;
        const std::shared_ptr<IceStorm::TraceLevels> _traceLevels;
        const std::shared_ptr<IceStormElection::Observers> _observers;
        const std::shared_ptr<Replica> _replica; // The replica.
        const Ice::ObjectPrx _replicaProxy;      // A proxy to the individual replica.

        const int _id;                             // My node id.
        const std::map<int, NodePrx> _nodes;       // The nodes indexed by their id.
        const std::map<int, NodePrx> _nodesOneway; // The nodes indexed by their id (as oneway proxies).

        const std::chrono::seconds _masterTimeout;
        const std::chrono::seconds _electionTimeout;
        const std::chrono::seconds _mergeTimeout;

        NodeState _state{NodeState::NodeStateInactive};
        int _updateCounter{0};

        int _coord;         // Id of the coordinator.
        std::string _group; // My group id.

        std::set<GroupNodeInfo> _up;    // Set of nodes in my group.
        std::set<int> _invitesIssued;   // The issued invitations.
        std::set<int> _invitesAccepted; // The accepted invitations.

        unsigned int _max{0};         // The highest group count I've seen.
        std::int64_t _generation{-1}; // The current generation (or -1 if not set).

        std::optional<Ice::ObjectPrx> _coordinatorProxy;
        bool _destroy{false};

        IceInternal::TimerTaskPtr _mergeTask;
        IceInternal::TimerTaskPtr _timeoutTask;
        IceInternal::TimerTaskPtr _checkTask;
        IceInternal::TimerTaskPtr _mergeContinueTask;

        mutable std::recursive_mutex _mutex;
        std::condition_variable_any _condVar;
    };

    class FinishUpdateHelper
    {
    public:
        FinishUpdateHelper(std::shared_ptr<NodeI> node) : _node(std::move(node)) {}

        ~FinishUpdateHelper()
        {
            if (_node)
            {
                _node->finishUpdate();
            }
        }

    private:
        const std::shared_ptr<NodeI> _node;
    };

    class CachedReadHelper
    {
    public:
        CachedReadHelper(std::shared_ptr<NodeI> node, const char* file, int line) : _node(std::move(node))
        {
            if (_node)
            {
                _master = _node->startCachedRead(_generation, file, line);
            }
        }

        ~CachedReadHelper()
        {
            if (_node)
            {
                _node->finishUpdate();
            }
        }

        [[nodiscard]] std::optional<Ice::ObjectPrx> getMaster() const { return _master; }

        [[nodiscard]] std::int64_t generation() const { return _generation; }

        [[nodiscard]] bool observerPrecondition(std::int64_t generation) const
        {
            return generation == _generation && _master;
        }

    private:
        const std::shared_ptr<NodeI> _node;
        std::optional<Ice::ObjectPrx> _master;
        std::int64_t _generation{-1};
    };

    class ObserverUpdateHelper
    {
    public:
        ObserverUpdateHelper(std::shared_ptr<NodeI> node, std::int64_t generation, const char* file, int line)
            : _node(std::move(node))
        {
            if (_node)
            {
                _node->startObserverUpdate(generation, file, line);
            }
        }

        ~ObserverUpdateHelper()
        {
            if (_node)
            {
                _node->finishUpdate();
            }
        }

    private:
        const std::shared_ptr<NodeI> _node;
    };
}

#endif // NODE_I_H
