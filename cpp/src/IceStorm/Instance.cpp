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
    lock_guard<mutex> lg(_mutex);
    _topics.push_back(name);
}

vector<string>
TopicReaper::consumeReapedTopics()
{
    lock_guard<mutex> lg(_mutex);
    vector<string> reaped;
    reaped.swap(_topics);
    return reaped;
}

Instance::Instance(const string& instanceName,
                   const string& name,
                   shared_ptr<Ice::Communicator> communicator,
                   shared_ptr<Ice::ObjectAdapter> publishAdapter,
                   shared_ptr<Ice::ObjectAdapter> topicAdapter,
                   shared_ptr<Ice::ObjectAdapter> nodeAdapter,
                   shared_ptr<NodePrx> nodeProxy) :
    _instanceName(instanceName),
    _serviceName(name),
    _communicator(move(communicator)),
    _publishAdapter(move(publishAdapter)),
    _topicAdapter(move(topicAdapter)),
    _nodeAdapter(move(nodeAdapter)),
    _nodeProxy(move(nodeProxy)),
    _traceLevels(make_shared<TraceLevels>(name, _communicator->getProperties(), _communicator->getLogger())),
    // default one minute.
    _discardInterval(_communicator->getProperties()->getPropertyAsIntWithDefault(name + ".Discard.Interval", 60)),
    // default one second.
    _flushInterval(_communicator->getProperties()->getPropertyAsIntWithDefault(name + ".Flush.Timeout", 1000)),
    // default one minute.
    _sendTimeout(_communicator->getProperties()->getPropertyAsIntWithDefault(name + ".Send.Timeout", 60 * 1000)),
    _sendQueueSizeMax(_communicator->getProperties()->getPropertyAsIntWithDefault(name + ".Send.QueueSizeMax", -1)),
    _sendQueueSizeMaxPolicy(RemoveSubscriber),
    _topicReaper(make_shared<TopicReaper>()),
    _observers(make_shared<Observers>(_traceLevels))
{
    try
    {
        auto properties = _communicator->getProperties();
        if(properties->getProperty(name + ".TopicManager.AdapterId").empty())
        {
            string p = properties->getProperty(name + ".ReplicatedTopicManagerEndpoints");
            if(!p.empty())
            {
                const_cast<shared_ptr<Ice::ObjectPrx>&>(_topicReplicaProxy) =
                    _communicator->stringToProxy("dummy:" + p);
            }
            p = properties->getProperty(name + ".ReplicatedPublishEndpoints");
            if(!p.empty())
            {
                const_cast<shared_ptr<Ice::ObjectPrx>&>(_publisherReplicaProxy) =
                    _communicator->stringToProxy("dummy:" + p);
            }
        }

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
        auto o = dynamic_pointer_cast<IceInternal::CommunicatorObserverI>(_communicator->getObserver());
        if(o)
        {
            _observer = make_shared<TopicManagerObserverI>(o->getFacet());
        }
    }
    catch(const std::exception&)
    {
        shutdown();
        destroy();
        throw;
    }
}

Instance::~Instance()
{
}

void
Instance::setNode(shared_ptr<NodeI> node)
{
    _node = move(node);
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

shared_ptr<Ice::Communicator>
Instance::communicator() const
{
    return _communicator;
}

shared_ptr<Ice::Properties>
Instance::properties() const
{
    return _communicator->getProperties();
}

shared_ptr<Ice::ObjectAdapter>
Instance::publishAdapter() const
{
    return _publishAdapter;
}

shared_ptr<Ice::ObjectAdapter>
Instance::topicAdapter() const
{
    return _topicAdapter;
}

shared_ptr<Ice::ObjectAdapter>
Instance::nodeAdapter() const
{
    return _nodeAdapter;
}

shared_ptr<Observers>
Instance::observers() const
{
    return _observers;
}

shared_ptr<NodeI>
Instance::node() const
{
    return _node;
}

shared_ptr<NodePrx>
Instance::nodeProxy() const
{
    return _nodeProxy;
}

shared_ptr<TraceLevels>
Instance::traceLevels() const
{
    return _traceLevels;
}

IceUtil::TimerPtr
Instance::timer() const
{
    return _timer;
}

shared_ptr<Ice::ObjectPrx>
Instance::topicReplicaProxy() const
{
    return _topicReplicaProxy;
}

shared_ptr<Ice::ObjectPrx>
Instance::publisherReplicaProxy() const
{
    return _publisherReplicaProxy;
}

shared_ptr<IceStorm::Instrumentation::TopicManagerObserver>
Instance::observer() const
{
    return _observer;
}

shared_ptr<IceStorm::TopicReaper>
Instance::topicReaper() const
{
    return _topicReaper;
}

chrono::seconds
Instance::discardInterval() const
{
    return _discardInterval;
}

chrono::milliseconds
Instance::flushInterval() const
{
    return _flushInterval;
}

chrono::milliseconds
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
    // The node instance must be cleared as the node holds the
    // replica (TopicManager) which holds the instance causing a
    // cyclic reference.
    _node = nullptr;
    //
    // The observer instance must be cleared as it holds the
    // TopicManagerImpl which holds the instance causing a
    // cyclic reference.
    //
    _observer = nullptr;
}

PersistentInstance::PersistentInstance(const string& instanceName,
                                       const string& name,
                                       shared_ptr<Ice::Communicator> communicator,
                                       shared_ptr<Ice::ObjectAdapter> publishAdapter,
                                       shared_ptr<Ice::ObjectAdapter> topicAdapter,
                                       shared_ptr<Ice::ObjectAdapter> nodeAdapter,
                                       shared_ptr<NodePrx> nodeProxy) :
    Instance(instanceName, name, communicator, move(publishAdapter), move(topicAdapter), move(nodeAdapter),
             move(nodeProxy)),
    _dbLock(communicator->getProperties()->getPropertyWithDefault(name + ".LMDB.Path", name) + "/icedb.lock"),
    _dbEnv(communicator->getProperties()->getPropertyWithDefault(name + ".LMDB.Path", name), 2,
        IceDB::getMapSize(communicator->getProperties()->getPropertyAsInt(name + ".LMDB.MapSize")))
{
    try
    {
        dbContext.communicator = move(communicator);
        dbContext.encoding.minor = 1;
        dbContext.encoding.major = 1;

        IceDB::ReadWriteTxn txn(_dbEnv);

        _lluMap = LLUMap(txn, "llu", dbContext, MDB_CREATE);
        _subscriberMap = SubscriberMap(txn, "subscribers", dbContext, MDB_CREATE, compareSubscriberRecordKey);

        txn.commit();
    }
    catch(const std::exception&)
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
    dbContext.communicator = nullptr;

    Instance::destroy();
}
