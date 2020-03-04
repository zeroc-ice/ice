//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceStorm/TraceLevels.h>

#include <Ice/Properties.h>

using namespace std;
using namespace IceStorm;

TraceLevels::TraceLevels(const string name,
                         const shared_ptr<Ice::Properties>& properties,
                         shared_ptr<Ice::Logger> theLogger) :
    topicMgr(0),
    topicMgrCat("TopicManager"),
    topic(0),
    topicCat("Topic"),
    subscriber(0),
    subscriberCat("Subscriber"),
    election(0),
    electionCat("Election"),
    replication(0),
    replicationCat("Replication"),
    logger(move(theLogger))
{
    const string keyBase = name + ".Trace.";
    const_cast<int&>(topicMgr) = properties->getPropertyAsInt(keyBase + topicMgrCat);
    const_cast<int&>(topic) = properties->getPropertyAsInt(keyBase + topicCat);
    const_cast<int&>(subscriber) = properties->getPropertyAsInt(keyBase + subscriberCat);
    const_cast<int&>(election) = properties->getPropertyAsInt(keyBase + electionCat);
}
