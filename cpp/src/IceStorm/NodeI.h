// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ELECTION_I_H
#define ELECTION_I_H

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <IceStorm/Election.h>
#include <IceStorm/Replica.h>
#include <IceStorm/Instance.h>
#include <IceUtil/Timer.h>
#include <set>

namespace IceStormElection
{

class Observers;
typedef IceUtil::Handle<Observers> ObserversPtr;

class NodeI : public Node, public IceUtil::Monitor<IceUtil::RecMutex>
{
public:

    NodeI(const IceStorm::InstancePtr&, const ReplicaPtr&, const Ice::ObjectPrx&,
          int, const std::map<int, NodePrx>&);

    void start();

    void check();
    void timeout();
    void merge(const std::set<int>&);
    void mergeContinue();
    virtual void invitation(int, const std::string&, const Ice::Current&);
    virtual void ready(int, const std::string&, const Ice::ObjectPrx&, int, Ice::Long, const Ice::Current&);
    virtual void accept(int, const std::string&, const Ice::IntSeq&, const Ice::ObjectPrx&, const LogUpdate&, int,
                        const Ice::Current&);
    virtual bool areYouCoordinator(const Ice::Current&) const;
    virtual bool areYouThere(const std::string&, int, const Ice::Current&) const;
    virtual Ice::ObjectPrx sync(const Ice::Current&) const;
    virtual NodeInfoSeq nodes(const Ice::Current&) const;
    virtual QueryInfo query(const Ice::Current&) const;
    void recovery(Ice::Long = -1);

    void destroy();

    // Notify the node that we're about to start an update.
    void checkObserverInit(Ice::Long);
    Ice::ObjectPrx startUpdate(Ice::Long&, const char*, int);
    Ice::ObjectPrx startCachedRead(Ice::Long&, const char*, int);
    void startObserverUpdate(Ice::Long, const char*, int);
    bool updateMaster(const char*, int);

    // The node has completed the update.
    void finishUpdate();

private:

    void setState(NodeState);

    const IceUtil::TimerPtr _timer;
    const IceStorm::TraceLevelsPtr _traceLevels;
    const IceStormElection::ObserversPtr _observers;
    const ReplicaPtr _replica; // The replica.
    const Ice::ObjectPrx _replicaProxy; // A proxy to the individual replica.

    const int _id; // My node id.
    const std::map<int, NodePrx> _nodes; // The nodes indexed by their id.
    const std::map<int, NodePrx> _nodesOneway; // The nodes indexed by their id (as oneway proxies).

    const IceUtil::Time _masterTimeout;
    const IceUtil::Time _electionTimeout;
    const IceUtil::Time _mergeTimeout;

    NodeState _state;
    int _updateCounter;

    int _coord; // Id of the coordinator.
    std::string _group; // My group id.

    std::set<GroupNodeInfo> _up; // Set of nodes in my group.
    std::set<int> _invitesIssued; // The issued invitations.
    std::set<int> _invitesAccepted; // The accepted invitations.

    unsigned int _max; // The highest group count I've seen.
    Ice::Long _generation; // The current generation (or -1 if not set).

    Ice::ObjectPrx _coordinatorProxy;
    bool _destroy;

    // Various timers.
    IceUtil::TimerTaskPtr _mergeTask;
    IceUtil::TimerTaskPtr _timeoutTask;
    IceUtil::TimerTaskPtr _checkTask;
    IceUtil::TimerTaskPtr _mergeContinueTask;
};
typedef IceUtil::Handle<NodeI> NodeIPtr;

class FinishUpdateHelper
{
public:

    FinishUpdateHelper(const NodeIPtr& node) :
        _node(node)
    {
    }

    ~FinishUpdateHelper()
    {
        if(_node)
        {
            _node->finishUpdate();
        }
    }

private:

    const NodeIPtr _node;
};

class CachedReadHelper
{
public:

    CachedReadHelper(const NodeIPtr& node, const char* file, int line) :
        _node(node)
    {
        if(_node)
        {
            _master = _node->startCachedRead(_generation, file, line);
        }
    }

    ~CachedReadHelper()
    {
        if(_node)
        {
            _node->finishUpdate();
        }
    }

    Ice::ObjectPrx
    getMaster() const
    {
        return _master;
    }

    Ice::Long
    generation() const
    {
        return _generation;
    }

    bool
    observerPrecondition(Ice::Long generation) const
    {
        return generation == _generation && _master;
    }

private:

    const NodeIPtr _node;
    Ice::ObjectPrx _master;
    Ice::Long _generation;
};

class ObserverUpdateHelper
{
public:

    ObserverUpdateHelper(const NodeIPtr& node, Ice::Long generation, const char* file, int line) :
        _node(node)
    {
        if(_node)
        {
            _node->startObserverUpdate(generation, file, line);
        }
    }

    ~ObserverUpdateHelper()
    {
        if(_node)
        {
            _node->finishUpdate();
        }
    }

private:

    const NodeIPtr _node;
};

}

#endif // ELECTION_I_H
