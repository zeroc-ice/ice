// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    flush(0),
    flushCat("Flush"),
    subscriberPool(0),
    subscriberPoolCat("SubscriberPool"),
    subscriber(0),
    subscriberCat("Subscriber"),
    logger(theLogger)
{
    const string keyBase = name + ".Trace.";
    const_cast<int&>(topicMgr) = properties->getPropertyAsInt(keyBase + topicMgrCat);
    const_cast<int&>(topic) = properties->getPropertyAsInt(keyBase + topicCat);
    const_cast<int&>(flush) = properties->getPropertyAsInt(keyBase + flushCat);
    const_cast<int&>(subscriberPool) = properties->getPropertyAsInt(keyBase + subscriberPoolCat);
    const_cast<int&>(subscriber) = properties->getPropertyAsInt(keyBase + subscriberCat);
}

TraceLevels::~TraceLevels()
{
}
