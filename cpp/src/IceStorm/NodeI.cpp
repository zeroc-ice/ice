//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceStorm/NodeI.h>
#include <IceStorm/Observers.h>
#include <IceStorm/TraceLevels.h>

using namespace IceStorm;
using namespace IceStormElection;
using namespace std;

namespace
{

class CheckTask : public IceUtil::TimerTask
{
    const NodeIPtr _node;

public:

    CheckTask(const NodeIPtr& node) : _node(node) { }
    virtual void runTimerTask()
    {
        _node->check();
    }
};

class MergeTask : public IceUtil::TimerTask
{
    const NodeIPtr _node;
    const set<int> _s;

public:

    MergeTask(const NodeIPtr& node, const set<int>& s) : _node(node), _s(s) { }
    virtual void runTimerTask()
    {
        _node->merge(_s);
    }
};

class MergeContinueTask : public IceUtil::TimerTask
{
    const NodeIPtr _node;

public:

    MergeContinueTask(const NodeIPtr& node) : _node(node) { }
    virtual void runTimerTask()
    {
        _node->mergeContinue();
    }
};

class TimeoutTask: public IceUtil::TimerTask
{
    const NodeIPtr _node;

public:

    TimeoutTask(const NodeIPtr& node) : _node(node) { }
    virtual void runTimerTask()
    {
        _node->timeout();
    }
};

}

namespace
{

LogUpdate emptyLU = {0, 0};

}

GroupNodeInfo::GroupNodeInfo(int i) :
    id(i), llu(emptyLU)
{
}

GroupNodeInfo::GroupNodeInfo(int i, LogUpdate l, const Ice::ObjectPrx& o) :
    id(i), llu(l), observer(o)
{
}

bool
GroupNodeInfo::operator<(const GroupNodeInfo& rhs) const
{
    return id < rhs.id;
}

bool
GroupNodeInfo::operator==(const GroupNodeInfo& rhs) const
{
    return id == rhs.id;
}

//
// COMPILER FIX: Clang using libc++ requires to define operator=
//
#if defined(__clang__) && defined(_LIBCPP_VERSION)
GroupNodeInfo&
GroupNodeInfo::operator=(const GroupNodeInfo& other)

{
    const_cast<int&>(this->id) = other.id;
    const_cast<LogUpdate&>(this->llu) = other.llu;
    const_cast<Ice::ObjectPrx&>(this->observer) = other.observer;
    return *this;
}
#endif

namespace
{
static IceUtil::Time
getTimeout(const string& key, int def, const Ice::PropertiesPtr& properties, const TraceLevelsPtr& traceLevels)
{
    int t = properties->getPropertyAsIntWithDefault(key, def);
    if(t < 0)
    {
        Ice::Warning out(traceLevels->logger);
        out << traceLevels->electionCat << ": " << key << " < 0; Adjusted to 1";
        t = 1;
    }
    return IceUtil::Time::seconds(t);
}

static string
toString(const set<int>& s)
{
    ostringstream os;
    os << "(";
    for(set<int>::const_iterator p = s.begin(); p != s.end(); ++p)
    {
        if(p != s.begin())
        {
            os << ",";
        }
        os << *p;
    }
    os << ")";
    return os.str();
}

}

NodeI::NodeI(const InstancePtr& instance,
             const ReplicaPtr& replica,
             const Ice::ObjectPrx& replicaProxy,
             int id, const map<int, NodePrx>& nodes) :
    _timer(instance->timer()),
    _traceLevels(instance->traceLevels()),
    _observers(instance->observers()),
    _replica(replica),
    _replicaProxy(replicaProxy),
    _id(id),
    _nodes(nodes),
    _state(NodeStateInactive),
    _updateCounter(0),
    _max(0),
    _generation(-1),
    _destroy(false)
{
    map<int, NodePrx> oneway;
    for(map<int, NodePrx>::const_iterator p = _nodes.begin(); p != _nodes.end(); ++p)
    {
        oneway[p->first] = NodePrx::uncheckedCast(p->second->ice_oneway());
    }
    const_cast<map<int, NodePrx>& >(_nodesOneway) = oneway;

    Ice::PropertiesPtr properties = instance->communicator()->getProperties();
    const_cast<IceUtil::Time&>(_masterTimeout) = getTimeout(
        instance->serviceName() + ".Election.MasterTimeout", 10, properties, _traceLevels);
    const_cast<IceUtil::Time&>(_electionTimeout) = getTimeout(
        instance->serviceName() + ".Election.ElectionTimeout", 10, properties, _traceLevels);
    const_cast<IceUtil::Time&>(_mergeTimeout) = getTimeout(
        instance->serviceName() + ".Election.ResponseTimeout", 10, properties, _traceLevels);
}

void
NodeI::start()
{
    // As an optimization we want the initial election to occur as
    // soon as possible.
    //
    // However, if we have the node trigger the election immediately
    // upon startup then we'll have a clash with lower priority nodes
    // starting an election denying a higher priority node the
    // opportunity to start the election that results in it becoming
    // the leader. Of course, things will eventually reach a stable
    // state but it will take longer.
    //
    // As such as we schedule the initial election check inversely
    // proportional to our priority.
    //
    // By setting _checkTask first we stop recovery() from setting it
    // to the regular election interval.
    //

    //
    // We use this lock to ensure that recovery is called before CheckTask
    // is scheduled, even if timeout is 0
    //
    Lock sync(*this);

    _checkTask = new CheckTask(this);
    _timer->schedule(_checkTask, IceUtil::Time::seconds((_nodes.size() - _id) * 2));
    recovery();
}

void
NodeI::check()
{
    {
        Lock sync(*this);
        if(_destroy)
        {
            return;
        }
        assert(!_mergeTask);

        if(_state == NodeStateElection || _state == NodeStateReorganization || _coord != _id)
        {
            assert(_checkTask);
            _timer->schedule(_checkTask, _electionTimeout);
            return;
        }

        // Next get the set of nodes that were detected as unreachable
        // from the replica and remove them from our slave list.
        vector<int> dead;
        _observers->getReapedSlaves(dead);
        if(!dead.empty())
        {
            for(vector<int>::const_iterator p = dead.begin(); p != dead.end(); ++p)
            {
                set<GroupNodeInfo>::iterator q = _up.find(GroupNodeInfo(*p));
                if(q != _up.end())
                {
                    if(_traceLevels->election > 0)
                    {
                        Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
                        out << "node " << _id << ": reaping slave " << *p;
                    }
                    _up.erase(q);
                }
            }

            // If we no longer have the majority of the nodes under our
            // care then we need to stop our replica.
            if(_up.size() < _nodes.size()/2)
            {
                if(_traceLevels->election > 0)
                {
                    Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
                    out << "node " << _id << ": stopping replica";
                }
                // Clear _checkTask -- recovery() will reset the
                // timer.
                assert(_checkTask);
                _checkTask = 0;
                recovery();
                return;
            }
        }
    }

    // See if other groups exist for possible merge.
    set<int> tmpset;
    int max = -1;
    for(map<int, NodePrx>::const_iterator p = _nodes.begin(); p != _nodes.end(); ++p)
    {
        if(p->first == _id)
        {
            continue;
        }
        try
        {
            if(p->second->areYouCoordinator())
            {
                if(p->first > max)
                {
                    max = p->first;
                }
                tmpset.insert(p->first);
            }
        }
        catch(const Ice::Exception& ex)
        {
            if(_traceLevels->election > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
                out << "node " << _id << ": call on node " << p->first << " failed: " << ex;
            }
        }
    }

    Lock sync(*this);

    // If the node state has changed while the mutex has been released
    // then bail. We don't schedule a re-check since we're either
    // destroyed in which case we're going to terminate or the end of
    // the election/reorg will re-schedule the check.
    if(_destroy || _state == NodeStateElection || _state == NodeStateReorganization || _coord != _id)
    {
        _checkTask = 0;
        return;
    }

    // If we didn't find any coordinators then we're done. Reschedule
    // the next check and terminate.
    if(tmpset.empty())
    {
        assert(_checkTask);
        _timer->schedule(_checkTask, _electionTimeout);
        return;
    }

    // _checkTask == 0 means that the check isn't scheduled.
    _checkTask = 0;

    if(_traceLevels->election > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
        out << "node " << _id << ": highest priority node count: " << max;
    }

    IceUtil::Time delay = IceUtil::Time::seconds(0);
    if(_id < max)
    {
        // Reschedule timer proportial to p-i.
        delay = _mergeTimeout + _mergeTimeout * (max - _id);
        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": scheduling merge in " << delay.toDuration()
                << " seconds";
        }
    }

    assert(!_mergeTask);
    _mergeTask = new MergeTask(this, tmpset);
    _timer->schedule(_mergeTask, delay);
}

// Called if the node has not heard from the coordinator in some time.
void
NodeI::timeout()
{
    int myCoord;
    string myGroup;
    {
        Lock sync(*this);
        // If we're destroyed or we are our own coordinator then we're
        // done.
        if(_destroy || _coord == _id)
        {
            return;
        }
        myCoord = _coord;
        myGroup = _group;
    }

    bool failed = false;
    try
    {
        map<int, NodePrx>::const_iterator p = _nodes.find(myCoord);
        assert(p != _nodes.end());
        if(!p->second->areYouThere(myGroup, _id))
        {
            if(_traceLevels->election > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
                out << "node " << _id << ": lost connection to coordinator " << myCoord
                    << ": areYouThere returned false";
            }
            failed = true;
        }
    }
    catch(const Ice::Exception& ex)
    {
        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": lost connection to coordinator " << myCoord << ": " << ex;
        }
        failed = true;
    }
    if(failed)
    {
        recovery();
    }
}

void
NodeI::merge(const set<int>& coordinatorSet)
{
    set<int> invited;
    string gp;
    {
        Lock sync(*this);
        _mergeTask = 0;

        // If the node is currently in an election, or reorganizing
        // then we're done.
        if(_state == NodeStateElection || _state == NodeStateReorganization)
        {
            return;
        }

        // This state change prevents this node from accepting
        // invitations while the merge is executing.
        setState(NodeStateElection);

        // No more replica changes are permitted.
        while(!_destroy && _updateCounter > 0)
        {
            wait();
        }
        if(_destroy)
        {
            return;
        }

        ostringstream os;
        os << _id << ":" << Ice::generateUUID();
        _group = os.str();
        gp = _group;

        _invitesAccepted.clear();
        _invitesIssued.clear();

        // Construct a set of node ids to invite. This is the union of
        // _up and set of coordinators gathered in the check stage.
        invited = coordinatorSet;
        for(set<GroupNodeInfo>::const_iterator p = _up.begin(); p != _up.end(); ++p)
        {
            invited.insert(p->id);
        }

        _coord = _id;
        _up.clear();

        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": inviting " << toString(invited) << " to group " << _group;
        }
    }

    set<int>::iterator p = invited.begin();
    while(p != invited.end())
    {
        try
        {
            if(_traceLevels->election > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
                out << "node " << _id << ": inviting node " << *p << " to group " << gp;
            }
            map<int, NodePrx>::const_iterator node = _nodesOneway.find(*p);
            assert(node != _nodesOneway.end());
            node->second->invitation(_id, gp);
            ++p;
        }
        catch(const Ice::Exception&)
        {
            invited.erase(p++);
        }
    }

    // Now we wait for responses to our invitation.
    {
        Lock sync(*this);
        if(_destroy)
        {
            return;
        }

        // Add each of the invited nodes in the invites issed set.
        _invitesIssued.insert(invited.begin(), invited.end());

        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": invites pending: " << toString(_invitesIssued);
        }

        // Schedule the mergeContinueTask.
        assert(_mergeContinueTask == 0);
        _mergeContinueTask = new MergeContinueTask(this);

        // At this point we may have already accepted all of the
        // invitations, if so then we want to schedule the
        // mergeContinue immediately.
        IceUtil::Time timeout = _mergeTimeout;
        if(_up.size() == _nodes.size()-1 || _invitesIssued == _invitesAccepted)
        {
            timeout = IceUtil::Time::seconds(0);
        }
        _timer->schedule(_mergeContinueTask, timeout);
    }
}

void
NodeI::mergeContinue()
{
    string gp;
    set<GroupNodeInfo> tmpSet;
    {
        Lock sync(*this);
        if(_destroy)
        {
            return;
        }

        // Copy variables for thread safety.
        gp = _group;
        tmpSet = _up;

        assert(_mergeContinueTask);
        _mergeContinueTask = 0;

        // The node is now reorganizing.
        assert(_state == NodeStateElection);
        setState(NodeStateReorganization);

        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": coordinator for " << (tmpSet.size() +1) << " nodes (including myself)";
        }

        // Now we need to decide whether we can start serving content. If
        // we're on initial startup then we need all nodes to participate
        // in the election. If we're running a subsequent election then we
        // need a majority of the nodes to be active in order to start
        // running.
        unsigned int ingroup = static_cast<unsigned int>(tmpSet.size());
        if((_max != _nodes.size() && ingroup != _nodes.size() -1) || ingroup < _nodes.size()/2)
        {
            if(_traceLevels->election > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
                out << "node " << _id << ": not enough nodes " << (ingroup+1) << "/" << _nodes.size()
                     << " for replication to commence";
                if(_max != _nodes.size())
                {
                    out << " (require full participation for startup)";
                }
            }
            recovery();
            return;
        }
    }

    // Find out who has the highest available set of database
    // updates.
    int maxid = -1;
    LogUpdate maxllu = { -1, 0 };
    for(set<GroupNodeInfo>::const_iterator p = tmpSet.begin(); p != tmpSet.end(); ++p)
    {
        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node id=" << p->id << " llu=" << p->llu.generation << "/" << p->llu.iteration;
        }
        if(p->llu.generation > maxllu.generation ||
           (p->llu.generation == maxllu.generation && p->llu.iteration > maxllu.iteration))
        {
            maxid = p->id;
            maxllu = p->llu;
        }
    }

    LogUpdate myLlu = _replica->getLastLogUpdate();
    if(_traceLevels->election > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
        out << "node id=" << _id << " llu=" << myLlu.generation << "/" << myLlu.iteration;
    }

    // If its not us then we have to get the latest database data from
    // the replica with the latest set.
    //if(maxllu > _replica->getLastLogUpdate())
    if(maxllu > myLlu)
    {
        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": syncing database state with node " << maxid;
        }
        try
        {
            map<int, NodePrx>::const_iterator node = _nodes.find(maxid);
            assert(node != _nodes.end());
            _replica->sync(node->second->sync());
        }
        catch(const Ice::Exception& ex)
        {
            if(_traceLevels->election > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
                out << "node " << _id << ": syncing database state with node "
                     << maxid << " failed: " << ex;
            }
            recovery();
            return;
        }
    }
    else
    {
        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": I have the latest database state.";
        }
    }

    // At this point we've ensured that we have the latest database
    // state, as such we can set our _max flag.
    unsigned int max = static_cast<unsigned int>(tmpSet.size()) + 1;
    {
        Lock sync(*this);
        if(max > _max)
        {
            _max = max;
        }
        max = _max;
    }

    // Prepare the LogUpdate for this generation.
    maxllu.generation++;
    maxllu.iteration = 0;

    try
    {
        // Tell the replica that it is now the master with the given
        // set of slaves and llu generation.
        _replica->initMaster(tmpSet, maxllu);
    }
    catch(const Ice::Exception& ex)
    {
        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": initMaster failed: " << ex;
        }
        recovery();
        return;
    }

    // Tell each node to go.
    for(set<GroupNodeInfo>::const_iterator p = tmpSet.begin(); p != tmpSet.end(); ++p)
    {
        try
        {
            map<int, NodePrx>::const_iterator node = _nodes.find(p->id);
            assert(node != _nodes.end());
            node->second->ready(_id, gp, _replicaProxy, max, maxllu.generation);
        }
        catch(const Ice::Exception& ex)
        {
            if(_traceLevels->election > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
                out << "node " << _id << ": error calling ready on " << p->id << " ex: " << ex;
            }
            recovery();
            return;
        }
    }

    {
        Lock sync(*this);
        if(_destroy)
        {
            return;
        }
        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": reporting for duty in group " << _group << " as coordinator. ";
            out << "replication commencing with " << _up.size()+1 << "/" << _nodes.size()
                << " nodes with llu generation: " << maxllu.generation;
        }
        setState(NodeStateNormal);
        _coordinatorProxy = 0;

        _generation = maxllu.generation;

        assert(!_checkTask);
        _checkTask = new CheckTask(this);
        _timer->schedule(_checkTask, _electionTimeout);
    }
}

void
NodeI::invitation(int j, const string& gn, const Ice::Current&)
{
    if(_traceLevels->election > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
        out << "node " << _id << ": invitation from " << j << " to group " << gn;
    }

    // Verify that j exists in our node set.
    if(_nodes.find(j) == _nodes.end())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << _traceLevels->electionCat << ": ignoring invitation from unknown node " << j;
        return;
    }

    int tmpCoord = -1;
    int max = -1;
    set<GroupNodeInfo> tmpSet;
    {
        Lock sync(*this);
        if(_destroy)
        {
            return;
        }
        // If we're in the election or reorg state a merge has already
        // started, so ignore the invitation.
        if(_state == NodeStateElection || _state == NodeStateReorganization)
        {
            if(_traceLevels->election > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
                out << "node " << _id << ": invitation ignored";
            }
            return;
        }

        //
        // Upon receipt of an invitation we cancel any pending merge
        // task.
        //
        if(_mergeTask)
        {
            // If the timer doesn't cancel it means that the timer has
            // fired and the merge is currently in-progress in which
            // case we should reject the invitation.
            if(!_timer->cancel(_mergeTask))
            {
                // The merge task is cleared in the merge. This
                // ensures two invitations cannot cause a race with
                // the merge.
                //_mergeTask = 0;
                return;
            }
            _mergeTask = 0;
        }

        // We're now joining with another group. If we are active we
        // must stop serving as a master or slave.
        setState(NodeStateElection);
        while(!_destroy && _updateCounter > 0)
        {
            wait();
        }
        if(_destroy)
        {
            return;
        }

        tmpCoord = _coord;
        tmpSet = _up;

        _coord = j;
        _group = gn;
        max = _max;
    }

    Ice::IntSeq forwardedInvites;
    if(tmpCoord == _id) // Forward invitation to my old members.
    {
        for(set<GroupNodeInfo>::const_iterator p = tmpSet.begin(); p != tmpSet.end(); ++p)
        {
            try
            {
                map<int, NodePrx>::const_iterator node = _nodesOneway.find(p->id);
                assert(node != _nodesOneway.end());
                node->second->invitation(j, gn);
                forwardedInvites.push_back(p->id);
            }
            catch(const Ice::Exception&)
            {
            }
        }
    }

    // Set the state and timer before calling accept. This ensures
    // that if ready is called directly after accept is called then
    // everything is fine. Setting the state *after* calling accept
    // can cause a race.
    {
        Lock sync(*this);
        if(_destroy)
        {
            return;
        }
        assert(_state == NodeStateElection);
        setState(NodeStateReorganization);
        if(!_timeoutTask)
        {
            _timeoutTask = new TimeoutTask(this);
            _timer->scheduleRepeated(_timeoutTask, _masterTimeout);
        }
    }

    try
    {
        map<int, NodePrx>::const_iterator node = _nodesOneway.find(j);
        assert(node != _nodesOneway.end());
        node->second->accept(_id, gn, forwardedInvites, _replica->getObserver(), _replica->getLastLogUpdate(), max);
    }
    catch(const Ice::Exception&)
    {
        recovery();
        return;
    }
}

void
NodeI::ready(int j, const string& gn, const Ice::ObjectPrx& coordinator, int max, Ice::Long generation,
             const Ice::Current&)
{
    Lock sync(*this);
    if(!_destroy && _state == NodeStateReorganization && _group == gn)
    {
        // The coordinator must be j (this was set in the invitation).
        if(_coord != j)
        {
            Ice::Warning warn(_traceLevels->logger);
            warn << _traceLevels->electionCat << ": ignoring ready call from replica node " << j
                 << " (real coordinator is " << _coord << ")";
            return;
        }

        // Here we've already validated j in the invite call
        // (otherwise _group != gn).
        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": reporting for duty in group " << gn << " with coordinator " << j;
        }

        if(static_cast<unsigned int>(max) > _max)
        {
            _max = max;
        }
        _generation = generation;

        // Activate the replica here since the replica is now ready
        // for duty.
        setState(NodeStateNormal);
        _coordinatorProxy = coordinator;

        if(!_checkTask)
        {
            _checkTask = new CheckTask(this);
            _timer->schedule(_checkTask, _electionTimeout);
        }
    }
}

void
NodeI::accept(int j, const string& gn, const Ice::IntSeq& forwardedInvites, const Ice::ObjectPrx& observer,
              const LogUpdate& llu, int max, const Ice::Current&)
{
    // Verify that j exists in our node set.
    if(_nodes.find(j) == _nodes.end())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << _traceLevels->electionCat << ": ignoring accept from unknown node " << j;
        return;
    }

    Lock sync(*this);
    if(!_destroy && _state == NodeStateElection && _group == gn && _coord == _id)
    {
        _up.insert(GroupNodeInfo(j, llu, observer));

        if(static_cast<unsigned int>(max) > _max)
        {
            _max = max;
        }

        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": accept " << j << " forward invites (";
            for(Ice::IntSeq::const_iterator p = forwardedInvites.begin(); p != forwardedInvites.end(); ++p)
            {
                if(p != forwardedInvites.begin())
                {
                    out << ",";
                }
                out << *p;
            }
            out << ") with llu "
                << llu.generation << "/" << llu.iteration << " into group " << gn
                << " group size " << (_up.size() + 1);
        }

        // Add each of the forwarded invites to the list of issued
        // invitations. This doesn't use set_union since
        // forwardedInvites may not be sorted.
        _invitesIssued.insert(forwardedInvites.begin(), forwardedInvites.end());
        // We've accepted the invitation from node j.
        _invitesAccepted.insert(j);

        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": invites pending: " << toString(_invitesIssued)
                << " invites accepted: " << toString(_invitesAccepted);
        }

        // If invitations have been accepted from all nodes and the
        // merge task has already been scheduled then reschedule the
        // merge continue immediately. Otherwise, we let the existing
        // merge() schedule continue.
        if((_up.size() == _nodes.size()-1 || _invitesIssued == _invitesAccepted) &&
           _mergeContinueTask && _timer->cancel(_mergeContinueTask))
        {
            _timer->schedule(_mergeContinueTask, IceUtil::Time::seconds(0));
        }
    }
}

bool
NodeI::areYouCoordinator(const Ice::Current&) const
{
    Lock sync(*this);
    return _state != NodeStateElection && _state != NodeStateReorganization && _coord == _id;
}

bool
NodeI::areYouThere(const string& gn, int j, const Ice::Current&) const
{
    Lock sync(*this);
    return _group == gn && _coord == _id && _up.find(GroupNodeInfo(j)) != _up.end();
}

Ice::ObjectPrx
NodeI::sync(const Ice::Current&) const
{
    return _replica->getSync();
}

NodeInfoSeq
NodeI::nodes(const Ice::Current&) const
{
    NodeInfoSeq seq;
    for(map<int, NodePrx>::const_iterator q = _nodes.begin(); q != _nodes.end(); ++q)
    {
        NodeInfo ni;
        ni.id = q->first;
        ni.n = q->second;
        seq.push_back(ni);
    }

    return seq;
}

QueryInfo
NodeI::query(const Ice::Current&) const
{
    Lock sync(*this);
    QueryInfo info;
    info.id = _id;
    info.coord = _coord;
    info.group = _group;
    info.replica = _replicaProxy;
    info.state = _state;
    info.max = _max;

    for(set<GroupNodeInfo>::const_iterator p = _up.begin(); p != _up.end(); ++p)
    {
        GroupInfo gi;
        gi.id = p->id;
        gi.llu = p->llu;
        info.up.push_back(gi);
    }

    return info;
}

void
NodeI::recovery(Ice::Long generation)
{
    Lock sync(*this);

    // Ignore the recovery if the node has already advanced a
    // generation.
    if(generation != -1 && generation != _generation)
    {
        return;
    }

    setState(NodeStateInactive);
    while(!_destroy && _updateCounter > 0)
    {
        wait();
    }
    if(_destroy)
    {
        return;
    }

    ostringstream os;
    os << _id << ":" << Ice::generateUUID();
    _group = os.str();

    _generation = -1;
    _coord = _id;
    _up.clear();

    if(_traceLevels->election > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
        out << "node " << _id << ": creating new self-coordinated group " << _group;
    }

    // Reset the timer states.
    if(_mergeTask)
    {
        _timer->cancel(_mergeTask);
        _mergeTask = 0;
    }
    if(_timeoutTask)
    {
        _timer->cancel(_timeoutTask);
        _timeoutTask = 0;
    }
    if(!_checkTask)
    {
        _checkTask = new CheckTask(this);
        _timer->schedule(_checkTask, _electionTimeout);
    }
}

void
NodeI::destroy()
{
    Lock sync(*this);
    assert(!_destroy);

    while(_updateCounter > 0)
    {
        wait();
    }
    _destroy = true;
    notifyAll();

    // Cancel the timers.
    if(_checkTask)
    {
        _timer->cancel(_checkTask);
        _checkTask = 0;
    }

    if(_timeoutTask)
    {
        _timer->cancel(_timeoutTask);
        _timeoutTask = 0;
    }

    if(_mergeTask)
    {
        _timer->cancel(_mergeTask);
        _mergeTask = 0;
    }
}

// A node should only receive an observer init call if the node is
// reorganizing and its not the coordinator.
void
NodeI::checkObserverInit(Ice::Long /*generation*/)
{
    Lock sync(*this);
    if(_state != NodeStateReorganization)
    {
        throw ObserverInconsistencyException("init cannot block when state != NodeStateReorganization");
    }
    if(_coord == _id)
    {
        throw ObserverInconsistencyException("init called on coordinator");
    }
}

// Notify the node that we're about to start an update.
Ice::ObjectPrx
NodeI::startUpdate(Ice::Long& generation, const char* file, int line)
{
    bool majority = _observers->check();

    Lock sync(*this);

    // If we've actively replicating & lost the majority of our replicas then recover.
    if(!_coordinatorProxy && !_destroy && _state == NodeStateNormal && !majority)
    {
        recovery();
    }

    while(!_destroy && _state != NodeStateNormal)
    {
        wait();
    }
    if(_destroy)
    {
        throw Ice::UnknownException(file, line);
    }
    if(!_coordinatorProxy)
    {
        ++_updateCounter;
    }
    generation = _generation;
    return _coordinatorProxy;
}

bool
NodeI::updateMaster(const char* /*file*/, int /*line*/)
{
    bool majority = _observers->check();

    Lock sync(*this);

    // If the node is destroyed, or is not a coordinator then we're
    // done.
    if(_destroy || _coordinatorProxy)
    {
        return false;
    }

    // If we've lost the majority of our replicas then recover.
    if(_state == NodeStateNormal && !majority)
    {
        recovery();
    }

    // If we're not replicating then we're done.
    if(_state != NodeStateNormal)
    {
        return false;
    }

    // Otherwise adjust the update counter, and return success.
    ++_updateCounter;
    return true;
}

Ice::ObjectPrx
NodeI::startCachedRead(Ice::Long& generation, const char* file, int line)
{
    Lock sync(*this);
    while(!_destroy && _state != NodeStateNormal)
    {
        wait();
    }
    if(_destroy)
    {
        throw Ice::UnknownException(file, line);
    }
    generation = _generation;
    ++_updateCounter;
    return _coordinatorProxy;
}

void
NodeI::startObserverUpdate(Ice::Long generation, const char* file, int line)
{
    Lock sync(*this);
    if(_destroy)
    {
        throw Ice::UnknownException(file, line);
    }
    if(_state != NodeStateNormal)
    {
        throw ObserverInconsistencyException("update called on inactive node");
    }
    if(!_coordinatorProxy)
    {
        throw ObserverInconsistencyException("update called on the master");
    }
    if(generation != _generation)
    {
        throw ObserverInconsistencyException("invalid generation");
    }
    ++_updateCounter;
}

void
NodeI::finishUpdate()
{
    Lock sync(*this);
    assert(!_destroy);
    --_updateCounter;
    assert(_updateCounter >= 0);
    if(_updateCounter == 0)
    {
        notifyAll();
    }
}

namespace
{
static string
stateToString(NodeState s)
{
    switch(s)
    {
    case NodeStateInactive:
        return "inactive";
    case NodeStateElection:
        return "election";
    case NodeStateReorganization:
        return "reorganization";
    case NodeStateNormal:
        return "normal";
    }
    return "unknown";
}
}

void
NodeI::setState(NodeState s)
{
    if(s != _state)
    {
        if(_traceLevels->election > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->electionCat);
            out << "node " << _id << ": transition from " << stateToString(_state) << " to "
                << stateToString(s);
        }
        _state = s;
        if(_state == NodeStateNormal)
        {
            notifyAll();
        }
    }
}
