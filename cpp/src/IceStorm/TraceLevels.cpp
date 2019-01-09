// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceStorm/TraceLevels.h>

#include <Ice/Properties.h>

using namespace std;
using namespace IceStorm;

TraceLevels::TraceLevels(const string name, const Ice::PropertiesPtr& properties, const Ice::LoggerPtr& theLogger) :
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
    logger(theLogger)
{
    const string keyBase = name + ".Trace.";
    const_cast<int&>(topicMgr) = properties->getPropertyAsInt(keyBase + topicMgrCat);
    const_cast<int&>(topic) = properties->getPropertyAsInt(keyBase + topicCat);
    const_cast<int&>(subscriber) = properties->getPropertyAsInt(keyBase + subscriberCat);
    const_cast<int&>(election) = properties->getPropertyAsInt(keyBase + electionCat);
}

TraceLevels::~TraceLevels()
{
}
