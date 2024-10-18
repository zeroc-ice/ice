//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TraceUtil.h"

using namespace std;
using namespace DataStormI;

TraceLevels::TraceLevels(const Ice::PropertiesPtr& properties, const Ice::LoggerPtr& logger)
    : topic(properties->getIcePropertyAsInt("DataStorm.Trace.Topic")),
      topicCat("Topic"),
      data(properties->getIcePropertyAsInt("DataStorm.Trace.Data")),
      dataCat("Data"),
      session(properties->getIcePropertyAsInt("DataStorm.Trace.Session")),
      sessionCat("Session"),
      logger(logger)
{
}
