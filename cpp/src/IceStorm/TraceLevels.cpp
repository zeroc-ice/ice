// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Properties.h>
#include <IceStorm/TraceLevels.h>

using namespace std;
using namespace IceStorm;

TraceLevels::TraceLevels(const string name, const Ice::PropertiesPtr& properties, const Ice::LoggerPtr& theLogger) :
    topicMgr(0),
    topicMgrCat("TopicManager"),
    topic(0),
    topicCat("Topic"),
    flush(0),
    flushCat("Flush"),
    subscriber(0),
    subscriberCat("Subscriber"),
    logger(theLogger)
{
    const string keyBase = name + ".Trace.";
    const_cast<int&>(topicMgr) = properties->getPropertyAsInt(keyBase + topicMgrCat);
    const_cast<int&>(topic) = properties->getPropertyAsInt(keyBase + topicCat);
    const_cast<int&>(flush) = properties->getPropertyAsInt(keyBase + flushCat);
    const_cast<int&>(subscriber) = properties->getPropertyAsInt(keyBase + subscriberCat);
}

TraceLevels::~TraceLevels()
{
}
