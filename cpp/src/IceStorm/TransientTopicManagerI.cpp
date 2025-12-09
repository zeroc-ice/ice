// Copyright (c) ZeroC, Inc.

#include "TransientTopicManagerI.h"
#include "Ice/Ice.h"
#include "Instance.h"
#include "Subscriber.h"
#include "TraceLevels.h"
#include "TransientTopicI.h"

#include <functional>

using namespace IceStorm;
using namespace std;

TransientTopicManagerImpl::TransientTopicManagerImpl(shared_ptr<Instance> instance) : _instance(std::move(instance)) {}

optional<TopicPrx>
TransientTopicManagerImpl::create(string name, const Ice::Current&)
{
    lock_guard lock(_mutex);

    reap();

    if (_topics.find(name) != _topics.end())
    {
        throw TopicExists(name);
    }

    return createImpl(std::move(name));
}

optional<TopicPrx>
TransientTopicManagerImpl::retrieve(string name, const Ice::Current&)
{
    lock_guard lock(_mutex);

    reap();

    auto p = _topics.find(name);
    if (p == _topics.end())
    {
        throw NoSuchTopic(name);
    }

    return _instance->topicAdapter()->createProxy<TopicPrx>(p->second->id());
}

optional<TopicPrx>
TransientTopicManagerImpl::createOrRetrieve(string name, const Ice::Current&)
{
    lock_guard lock(_mutex);

    reap();

    auto p = _topics.find(name);
    if (p == _topics.end())
    {
        return createImpl(std::move(name));
    }

    return _instance->topicAdapter()->createProxy<TopicPrx>(p->second->id());
}

TopicDict
TransientTopicManagerImpl::retrieveAll(const Ice::Current&)
{
    lock_guard lock(_mutex);

    reap();

    TopicDict all;
    for (const auto& topic : _topics)
    {
        all.insert({topic.first, _instance->topicAdapter()->createProxy<TopicPrx>(topic.second->id())});
    }

    return all;
}

optional<IceStormElection::NodePrx>
TransientTopicManagerImpl::getReplicaNode(const Ice::Current&) const
{
    return nullopt;
}

void
TransientTopicManagerImpl::reap()
{
    //
    // Must be called called with mutex locked.
    //
    for (const string& topic : _instance->topicReaper()->consumeReapedTopics())
    {
        auto i = _topics.find(topic);
        if (i != _topics.end() && i->second->destroyed())
        {
            auto id = i->second->id();
            auto traceLevels = _instance->traceLevels();
            if (traceLevels->topicMgr > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->topicMgrCat);
                out << "Reaping " << i->first;
            }

            try
            {
                _instance->topicAdapter()->remove(id);
            }
            catch (const Ice::ObjectAdapterDestroyedException&)
            {
                // Ignore
            }

            _topics.erase(i);
        }
    }
}

void
TransientTopicManagerImpl::shutdown()
{
    lock_guard lock(_mutex);
    for (const auto& topic : _topics)
    {
        topic.second->shutdown();
    }
}

optional<TopicPrx>
TransientTopicManagerImpl::createImpl(string name)
{
    // Called with _mutex locked.

    Ice::Identity id = IceStormInternal::nameToIdentity(_instance, name);

    auto traceLevels = _instance->traceLevels();
    if (traceLevels->topicMgr > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicMgrCat);
        out << "creating new topic \"" << name << "\". id: " << _instance->communicator()->identityToString(id);
    }

    //
    // Create topic implementation
    //
    auto topicImpl = TransientTopicImpl::create(_instance, name, id);

    //
    // The identity is the name of the Topic.
    //
    auto prx = _instance->topicAdapter()->add<TopicPrx>(topicImpl, id);
    _topics.insert({std::move(name), std::move(topicImpl)});
    return prx;
}
