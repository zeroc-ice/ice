// Copyright (c) ZeroC, Inc.

#include "TraceLevels.h"
#include "Ice/Properties.h"

using namespace std;
using namespace IceStorm;

TraceLevels::TraceLevels(const Ice::PropertiesPtr& properties, Ice::LoggerPtr theLogger)
    : topicMgr(properties->getIcePropertyAsInt("IceStorm.Trace.TopicManager")),
      topicMgrCat("TopicManager"),
      topic(properties->getIcePropertyAsInt("IceStorm.Trace.Topic")),
      topicCat("Topic"),
      subscriber(properties->getIcePropertyAsInt("IceStorm.Trace.Subscriber")),
      subscriberCat("Subscriber"),
      election(properties->getIcePropertyAsInt("IceStorm.Trace.Election")),
      electionCat("Election"),
      replication(properties->getIcePropertyAsInt("IceStorm.Trace.Replication")),
      replicationCat("Replication"),
      logger(std::move(theLogger))
{
}
