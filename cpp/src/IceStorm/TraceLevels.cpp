// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Properties.h>
#include <IceStorm/TraceLevels.h>

using namespace std;
using namespace IceStorm;

TraceLevels::TraceLevels(const Ice::PropertiesPtr& properties, const Ice::LoggerPtr& thelogger) :
    topicMgrCat("TopicManager"),
    topicCat("Topic"),
    flushCat("Flush"),
    subscriberCat("Subscriber"),
    logger(thelogger)
{
    const string keyBase = "IceStorm.Trace.";
    const_cast<int&>(topicMgr) = properties->getPropertyAsInt(keyBase + topicMgrCat);
    const_cast<int&>(topic) = properties->getPropertyAsInt(keyBase + topicCat);
    const_cast<int&>(flush) = properties->getPropertyAsInt(keyBase + flushCat);
    const_cast<int&>(subscriber) = properties->getPropertyAsInt(keyBase + subscriberCat);
}

TraceLevels::~TraceLevels()
{
}
