// Copyright (c) ZeroC, Inc.

#include "Instance.h"
#include "../Ice/InstrumentationI.h"
#include "../Ice/Timer.h"
#include "../Ice/TraceUtil.h"
#include "Ice/Communicator.h"
#include "Ice/Properties.h"
#include "InstrumentationI.h"
#include "NodeI.h"
#include "Observers.h"
#include "TraceLevels.h"

using namespace std;
using namespace IceStorm;
using namespace IceStormElection;
using namespace IceStormInternal;

namespace
{
    string getLMDBPath(const Ice::PropertiesPtr& properties, const string& serviceName)
    {
        string path = properties->getIceProperty("IceStorm.LMDB.Path");
        if (path.empty())
        {
            path = serviceName;
        }
        return path;
    }
}

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

Instance::Instance(
    string instanceName,
    const string& serviceName,
    shared_ptr<Ice::Communicator> communicator,
    Ice::ObjectAdapterPtr publishAdapter,
    Ice::ObjectAdapterPtr topicAdapter,
    Ice::ObjectAdapterPtr nodeAdapter,
    optional<NodePrx> nodeProxy)
    : _instanceName(std::move(instanceName)),
      _serviceName(serviceName),
      _communicator(std::move(communicator)),
      _publishAdapter(std::move(publishAdapter)),
      _topicAdapter(std::move(topicAdapter)),
      _nodeAdapter(std::move(nodeAdapter)),
      _nodeProxy(std::move(nodeProxy)),
      _traceLevels(make_shared<TraceLevels>(_communicator->getProperties(), _communicator->getLogger())),
      // default one minute.
      _discardInterval(_communicator->getProperties()->getIcePropertyAsInt("IceStorm.Discard.Interval")),
      // default one second.
      _flushInterval(_communicator->getProperties()->getIcePropertyAsInt("IceStorm.Flush.Timeout")),
      // default one minute.
      _sendTimeout(_communicator->getProperties()->getIcePropertyAsInt("IceStorm.Send.Timeout")),
      _sendQueueSizeMax(_communicator->getProperties()->getIcePropertyAsInt("IceStorm.Send.QueueSizeMax")),
      _topicReaper(make_shared<TopicReaper>()),
      _observers(make_shared<Observers>(_traceLevels))
{
    try
    {
        auto properties = _communicator->getProperties();
        if (properties->getIceProperty("IceStorm.TopicManager.AdapterId").empty())
        {
            string p = properties->getIceProperty("IceStorm.ReplicatedTopicManagerEndpoints");
            if (!p.empty())
            {
                const_cast<optional<Ice::ObjectPrx>&>(_topicReplicaProxy) = Ice::ObjectPrx{_communicator, "dummy:" + p};
            }
            p = properties->getIceProperty("IceStorm.ReplicatedPublishEndpoints");
            if (!p.empty())
            {
                const_cast<optional<Ice::ObjectPrx>&>(_publisherReplicaProxy) =
                    Ice::ObjectPrx{_communicator, "dummy:" + p};
            }
        }

        _timer = make_shared<IceInternal::Timer>();

        string policy = properties->getIceProperty("IceStorm.Send.QueueSizeMaxPolicy");
        if (policy == "RemoveSubscriber")
        {
            const_cast<SendQueueSizeMaxPolicy&>(_sendQueueSizeMaxPolicy) = RemoveSubscriber;
        }
        else if (policy == "DropEvents")
        {
            const_cast<SendQueueSizeMaxPolicy&>(_sendQueueSizeMaxPolicy) = DropEvents;
        }
        else if (!policy.empty())
        {
            Ice::Warning warn(_traceLevels->logger);
            warn << "invalid value '" << policy << "' for '" << serviceName << ".Send.QueueSizeMaxPolicy'";
        }

        //
        // If an Ice metrics observer is setup on the communicator, also
        // enable metrics for IceStorm.
        //
        auto o = dynamic_pointer_cast<IceInternal::CommunicatorObserverI>(_communicator->getObserver());
        if (o)
        {
            _observer = make_shared<TopicManagerObserverI>(o->getFacet());
        }
    }
    catch (const std::exception&)
    {
        shutdown();
        destroy();
        throw;
    }
}

Instance::~Instance()
{
    // Usually they are called before destruction, but in some cases they are not, for example when the constructor
    // of PersistentInstance fails because the database directory does not exist.
    shutdown();
    destroy(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
}

void
Instance::setNode(shared_ptr<NodeI> node)
{
    _node = std::move(node);
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

optional<NodePrx>
Instance::nodeProxy() const
{
    return _nodeProxy;
}

shared_ptr<TraceLevels>
Instance::traceLevels() const
{
    return _traceLevels;
}

IceInternal::TimerPtr
Instance::timer() const
{
    return _timer;
}

optional<Ice::ObjectPrx>
Instance::topicReplicaProxy() const
{
    return _topicReplicaProxy;
}

optional<Ice::ObjectPrx>
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
Instance::shutdown() noexcept
{
    if (_node)
    {
        _node->destroy();
        assert(_nodeAdapter);
        _nodeAdapter->destroy();
    }

    _topicAdapter->destroy();
    _publishAdapter->destroy();

    if (_timer)
    {
        _timer->destroy();
    }
}

void
Instance::destroy() noexcept
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

PersistentInstance::PersistentInstance(
    const string& instanceName,
    const string& serviceName,
    shared_ptr<Ice::Communicator> communicator,
    Ice::ObjectAdapterPtr publishAdapter,
    Ice::ObjectAdapterPtr topicAdapter,
    Ice::ObjectAdapterPtr nodeAdapter,
    optional<NodePrx> nodeProxy)
    : Instance(
          instanceName,
          serviceName,
          communicator,
          std::move(publishAdapter),
          std::move(topicAdapter),
          std::move(nodeAdapter),
          std::move(nodeProxy)),
      _dbLock(getLMDBPath(communicator->getProperties(), serviceName) + "/icedb.lock"),
      _dbEnv(
          getLMDBPath(communicator->getProperties(), serviceName),
          2,
          IceDB::getMapSize(communicator->getProperties()->getIcePropertyAsInt("IceStorm.LMDB.MapSize")))
{
    try
    {
        dbContext.communicator = std::move(communicator);

        IceDB::ReadWriteTxn txn(_dbEnv);

        _lluMap = LLUMap(txn, "llu", dbContext, MDB_CREATE);
        _subscriberMap = SubscriberMap(txn, "subscribers", dbContext, MDB_CREATE, compareSubscriberRecordKey);

        txn.commit();
    }
    catch (const std::exception&)
    {
        shutdown();
        destroy();

        throw;
    }
}

void
PersistentInstance::destroy() noexcept
{
    _dbEnv.close();
    dbContext.communicator = nullptr;

    Instance::destroy();
}
