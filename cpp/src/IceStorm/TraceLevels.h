//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESTORM_TRACE_LEVELS_H
#define ICESTORM_TRACE_LEVELS_H

#include "Ice/Logger.h"
#include "Ice/PropertiesF.h"

#include <string>

namespace IceStorm
{
    class TraceLevels // NOLINT:clang-analyzer-optin.performance.Padding
    {
    public:
        TraceLevels(const Ice::PropertiesPtr&, Ice::LoggerPtr);

        const int topicMgr;
        const char* topicMgrCat;

        const int topic;
        const char* topicCat;

        const int subscriber;
        const char* subscriberCat;

        const int election;
        const char* electionCat;

        const int replication;
        const char* replicationCat;

        const Ice::LoggerPtr logger;
    };

} // End namespace IceStorm

#endif
