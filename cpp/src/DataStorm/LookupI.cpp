//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "LookupI.h"
#include "Instance.h"
#include "NodeSessionManager.h"
#include "TopicFactoryI.h"

using namespace std;
using namespace DataStormContract;
using namespace DataStormI;

LookupI::LookupI(
    shared_ptr<NodeSessionManager> nodeSessionManager,
    shared_ptr<TopicFactoryI> topicFactory,
    NodePrx nodePrx)
    : _nodeSessionManager(std::move(nodeSessionManager)),
      _topicFactory(std::move(topicFactory)),
      _nodePrx(std::move(nodePrx))
{
}

void
LookupI::announceTopicReader(string name, optional<NodePrx> proxy, const Ice::Current& current)
{
    Ice::checkNotNull(proxy, __FILE__, __LINE__, current);
    _nodeSessionManager->announceTopicReader(name, *proxy, current.con);
    _topicFactory->createSubscriberSession(name, *proxy, current.con);
}

void
LookupI::announceTopicWriter(string name, optional<NodePrx> proxy, const Ice::Current& current)
{
    Ice::checkNotNull(proxy, __FILE__, __LINE__, current);
    _nodeSessionManager->announceTopicWriter(name, *proxy, current.con);
    _topicFactory->createPublisherSession(name, *proxy, current.con);
}

void
LookupI::announceTopics(
    Ice::StringSeq readers,
    Ice::StringSeq writers,
    optional<NodePrx> proxy,
    const Ice::Current& current)
{
    Ice::checkNotNull(proxy, __FILE__, __LINE__, current);
    _nodeSessionManager->announceTopics(readers, writers, *proxy, current.con);

    for (const auto& name : readers)
    {
        _topicFactory->createSubscriberSession(name, *proxy, current.con);
    }

    for (const auto& name : writers)
    {
        _topicFactory->createPublisherSession(name, *proxy, current.con);
    }
}

optional<NodePrx>
LookupI::createSession(optional<NodePrx> node, const Ice::Current& current)
{
    Ice::checkNotNull(node, __FILE__, __LINE__, current);
    _nodeSessionManager->createOrGet(std::move(*node), current.con, true);
    return _nodePrx;
}
