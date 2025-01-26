// Copyright (c) ZeroC, Inc.

#include "TraceLevels.h"
#include "Ice/Properties.h"

using namespace std;
using namespace IceStorm;

TraceLevels::TraceLevels(const Ice::PropertiesPtr& properties, Ice::LoggerPtr theLogger)
    : topicMgr(properties->getIcePropertyAsInt("IceStorm.Trace.TopicManager")),
      topic(properties->getIcePropertyAsInt("IceStorm.Trace.Topic")),
      subscriber(properties->getIcePropertyAsInt("IceStorm.Trace.Subscriber")),
      election(properties->getIcePropertyAsInt("IceStorm.Trace.Election")),
      replication(properties->getIcePropertyAsInt("IceStorm.Trace.Replication")),
      logger(std::move(theLogger))
{
}
