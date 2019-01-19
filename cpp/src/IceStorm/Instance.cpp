//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceStorm/Instance.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Observers.h>
#include <IceStorm/NodeI.h>
#include <IceStorm/InstrumentationI.h>
#include <IceUtil/Timer.h>

#include <Ice/InstrumentationI.h>
#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/TraceUtil.h>

using namespace std;
using namespace IceStorm;
using namespace IceStormElection;
using namespace IceStormInternal;

namespace IceStormInternal
{
extern IceDB::IceContext dbContext;
}

void
TopicReaper::add(const string& name)
{
    Lock sync(*this);
    _topics.push_back(name);
}

vector<string>
TopicReaper::consumeReapedTopics()
{
    Lock sync(*this);
    vector<string> reaped;
    reaped.swap(_topics);
    return reaped;
}

PersistentInstance::PersistentInstance(
    const string& instanceName,
    const string& name,
    const Ice::CommunicatorPtr& communicator,
    const Ice::ObjectAdapterPtr& publishAdapter,
    const Ice::ObjectAdapterPtr& topicAdapter,
    const Ice::ObjectAdapterPtr& nodeAdapter,
    const NodePrx& nodeProxy) :
    Instance(instanceName, name, communicator, publishAdapter, topicAdapter, nodeAdapter, nodeProxy),
    _dbLock(communicator->getProperties()->getPropertyWithDefault(name + ".LMDB.Path", name) + "/icedb.lock"),
    _dbEnv(communicator->getProperties()->getPropertyWithDefault(name + ".LMDB.Path", name), 2,
           IceDB::getMapSize(communicator->getProperties()->getPropertyAsInt(name + ".LMDB.MapSize")))
{
    try
    {
        dbContext.communicator = communicator;
        dbContext.encoding.minor = 1;
        dbContext.encoding.major = 1;

        IceDB::ReadWriteTxn txn(_dbEnv);

        _lluMap = LLUMap(txn, "llu", dbContext, MDB_CREATE);
        _subscriberMap = SubscriberMap(txn, "subscribers", dbContext, MDB_CREATE, compareSubscriberRecordKey);

        txn.commit();
    }
    catch(...)
    {
        shutdown();
        destroy();

        throw;
    }
}

void
PersistentInstance::destroy()
{
    _dbEnv.close();
    dbContext.communicator = 0;

    Instance::destroy();
}

Instance::Instance(
    const string& instanceName,
    const string& name,
    const Ice::CommunicatorPtr& communicator,
    const Ice::ObjectAdapterPtr& publishAdapter,
    const Ice::ObjectAdapterPtr& topicAdapter,
    const Ice::ObjectAdapterPtr& nodeAdapter,
    const NodePrx& nodeProxy) :
    _instanceName(instanceName),
    _serviceName(name),
    _communicator(communicator),
    _publishAdapter(publishAdapter),
    _topicAdapter(topicAdapter),
    _nodeAdapter(nodeAdapter),
    _nodeProxy(nodeProxy),
    _traceLevels(new TraceLevels(name, communicator->getProperties(), communicator->getLogger())),
    _discardInterval(IceUtil::Time::seconds(communicator->getProperties()->getPropertyAsIntWithDefault(
                                                name + ".Discard.Interval", 60))), // default one minute.
    _flushInterval(IceUtil::Time::milliSeconds(communicator->getProperties()->getPropertyAsIntWithDefault(
                                                   name + ".Flush.Timeout", 1000))), // default one second.
    // default one minute.
    _sendTimeout(communicator->getProperties()->getPropertyAsIntWithDefault(name + ".Send.Timeout", 60 * 1000)),
    _sendQueueSizeMax(communicator->getProperties()->getPropertyAsIntWithDefault(name + ".Send.QueueSizeMax", -1)),
    _sendQueueSizeMaxPolicy(RemoveSubscriber),
    _topicReaper(new TopicReaper())
{
    try
    {
        __setNoDelete(true);

        Ice::PropertiesPtr properties = communicator->getProperties();
        if(properties->getProperty(name + ".TopicManager.AdapterId").empty())
        {
            string p = properties->getProperty(name + ".ReplicatedTopicManagerEndpoints");
            if(!p.empty())
            {
                const_cast<Ice::ObjectPrx&>(_topicReplicaProxy) = communicator->stringToProxy("dummy:" + p);
            }
            p = properties->getProperty(name + ".ReplicatedPublishEndpoints");
            if(!p.empty())
            {
                const_cast<Ice::ObjectPrx&>(_publisherReplicaProxy) = communicator->stringToProxy("dummy:" + p);
            }
        }
        _observers = new Observers(this);
        _batchFlusher = new IceUtil::Timer();
        _timer = new IceUtil::Timer();

        string policy = properties->getProperty(name + ".Send.QueueSizeMaxPolicy");
        if(policy == "RemoveSubscriber")
        {
            const_cast<SendQueueSizeMaxPolicy&>(_sendQueueSizeMaxPolicy) = RemoveSubscriber;
        }
        else if(policy == "DropEvents")
        {
            const_cast<SendQueueSizeMaxPolicy&>(_sendQueueSizeMaxPolicy) = DropEvents;
        }
        else if(!policy.empty())
        {
            Ice::Warning warn(_traceLevels->logger);
            warn << "invalid value `" << policy << "' for `" << name << ".Send.QueueSizeMaxPolicy'";
        }

        //
        // If an Ice metrics observer is setup on the communicator, also
        // enable metrics for IceStorm.
        //
        IceInternal::CommunicatorObserverIPtr o =
            IceInternal::CommunicatorObserverIPtr::dynamicCast(communicator->getObserver());
        if(o)
        {
            _observer = new TopicManagerObserverI(o->getFacet());
        }
    }
    catch(...)
    {
        shutdown();
        destroy();
        __setNoDelete(false);

        throw;
    }
    __setNoDelete(false);
}

void
Instance::setNode(const NodeIPtr& node)
{
    _node = node;
}

string
Instance::instanceName() const
{
    return _instanceName;
}

string
Instance::serviceName() const
{
    return _serviceName;
}

Ice::CommunicatorPtr
Instance::communicator() const
{
    return _communicator;
}

Ice::PropertiesPtr
Instance::properties() const
{
    return _communicator->getProperties();
}

Ice::ObjectAdapterPtr
Instance::publishAdapter() const
{
    return _publishAdapter;
}

Ice::ObjectAdapterPtr
Instance::topicAdapter() const
{
    return _topicAdapter;
}

Ice::ObjectAdapterPtr
Instance::nodeAdapter() const
{
    return _nodeAdapter;
}

ObserversPtr
Instance::observers() const
{
    return _observers;
}

NodeIPtr
Instance::node() const
{
    return _node;
}

NodePrx
Instance::nodeProxy() const
{
    return _nodeProxy;
}

TraceLevelsPtr
Instance::traceLevels() const
{
    return _traceLevels;
}

IceUtil::TimerPtr
Instance::batchFlusher() const
{
    return _batchFlusher;
}

IceUtil::TimerPtr
Instance::timer() const
{
    return _timer;
}

Ice::ObjectPrx
Instance::topicReplicaProxy() const
{
    return _topicReplicaProxy;
}

Ice::ObjectPrx
Instance::publisherReplicaProxy() const
{
    return _publisherReplicaProxy;
}

IceStorm::Instrumentation::TopicManagerObserverPtr
Instance::observer() const
{
    return _observer;
}

IceStorm::TopicReaperPtr
Instance::topicReaper() const
{
    return _topicReaper;
}

IceUtil::Time
Instance::discardInterval() const
{
    return _discardInterval;
}

IceUtil::Time
Instance::flushInterval() const
{
    return _flushInterval;
}

int
Instance::sendTimeout() const
{
    return _sendTimeout;
}

int
Instance::sendQueueSizeMax() const
{
    return _sendQueueSizeMax;
}

Instance::SendQueueSizeMaxPolicy
Instance::sendQueueSizeMaxPolicy() const
{
    return _sendQueueSizeMaxPolicy;
}

void
Instance::shutdown()
{
    if(_node)
    {
        _node->destroy();
        assert(_nodeAdapter);
        _nodeAdapter->destroy();
    }

    _topicAdapter->destroy();
    _publishAdapter->destroy();

    if(_timer)
    {
        _timer->destroy();
    }
}

void
Instance::destroy()
{
    if(_batchFlusher)
    {
        _batchFlusher->destroy();
    }

    // The node instance must be cleared as the node holds the
    // replica (TopicManager) which holds the instance causing a
    // cyclic reference.
    _node = 0;
    //
    // The observer instance must be cleared as it holds the
    // TopicManagerImpl which hodlds the instance causing a
    // cyclic reference.
    //
    _observer = 0;
}
