// Copyright (c) ZeroC, Inc.

#include "LookupI.h"
#include "Instance.h"
#include "NodeSessionManager.h"
#include "TopicFactoryI.h"

using namespace std;
using namespace DataStormContract;
using namespace DataStormI;
using namespace Ice;

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
LookupI::announceTopicReader(string name, optional<NodePrx> subscriber, const Current& current)
{
    checkNotNull(subscriber, __FILE__, __LINE__, current);
    // Forward the announcement to known nodes via the node session manager.
    _nodeSessionManager->announceTopicReader(name, *subscriber, current.con);

    // Notify the topic factory about the new topic reader.
    // If there are any writers for the topic, the factory will create a subscriber session for it.
    _topicFactory->createSubscriberSession(name, *subscriber, current.con);
}

void
LookupI::announceTopicWriter(string name, optional<NodePrx> publisher, const Current& current)
{
    checkNotNull(publisher, __FILE__, __LINE__, current);
    // Forward the announcement to known nodes via the node session manager.
    _nodeSessionManager->announceTopicWriter(name, *publisher, current.con);

    // Notify the topic factory about the new topic writer.
    // If there are any readers for the topic, the factory will create a publisher session for it.
    _topicFactory->createPublisherSession(name, *publisher, current.con);
}

void
LookupI::announceTopics(StringSeq readers, StringSeq writers, optional<NodePrx> proxy, const Current& current)
{
    checkNotNull(proxy, __FILE__, __LINE__, current);
    // Forward the announcement to known nodes via the node session manager.
    _nodeSessionManager->announceTopics(readers, writers, *proxy, current.con);

    // Notify the topic factory about the new topic readers and writers.
    // The factory will create subscriber sessions for topics with matching writers and publisher sessions for topics
    // with matching readers.

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
LookupI::createSession(optional<NodePrx> node, const Current& current)
{
    checkNotNull(node, __FILE__, __LINE__, current);
    _nodeSessionManager->createOrGet(std::move(*node), current.con, true);
    return _nodePrx;
}
