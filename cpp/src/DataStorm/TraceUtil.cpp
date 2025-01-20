// Copyright (c) ZeroC, Inc.

#include "TraceUtil.h"

using namespace std;
using namespace DataStormI;
using namespace Ice;

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

TraceLevels::TraceLevels(const PropertiesPtr& properties, LoggerPtr logger)
    : topic(properties->getIcePropertyAsInt("DataStorm.Trace.Topic")),
      topicCat("Topic"),
      data(properties->getIcePropertyAsInt("DataStorm.Trace.Data")),
      dataCat("Data"),
      session(properties->getIcePropertyAsInt("DataStorm.Trace.Session")),
      sessionCat("Session"),
      logger(std::move(logger))
{
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
