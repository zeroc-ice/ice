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

void IceStorm::incRef(TraceLevels* p) { p->__incRef(); }
void IceStorm::decRef(TraceLevels* p) { p->__decRef(); }

TraceLevels::TraceLevels(const Ice::PropertiesPtr& properties) :
    topicMgr(0),
    topicMgrCat("TopicManager"),
    topic(0),
    topicCat("Topic"),
    flush(0),
    flushCat("Flush"),
    subscriber(0),
    subscriberCat("Subscriber")
{
    string value;
    const string keyBase = "IceStorm.Trace.";

    value = properties->getProperty(keyBase + topicMgrCat);
    if (!value.empty())
    {
	const_cast<int&>(topicMgr) = atoi(value.c_str());
    }

    value = properties->getProperty(keyBase + topicCat);
    if (!value.empty())
    {
	const_cast<int&>(topic) = atoi(value.c_str());
    }

    value = properties->getProperty(keyBase + flushCat);
    if (!value.empty())
    {
	const_cast<int&>(flush) = atoi(value.c_str());
    }

    value = properties->getProperty(keyBase + subscriberCat);
    if (!value.empty())
    {
	const_cast<int&>(subscriber) = atoi(value.c_str());
    }
}

TraceLevels::~TraceLevels()
{
}
