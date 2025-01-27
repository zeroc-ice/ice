// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_TRACE_LEVELS_H
#define ICESTORM_TRACE_LEVELS_H

#include "Ice/Logger.h"
#include "Ice/PropertiesF.h"

#include <string>

namespace IceStorm
{
    class TraceLevels // NOLINT(clang-analyzer-optin.performance.Padding)
    {
    public:
        TraceLevels(const Ice::PropertiesPtr&, Ice::LoggerPtr);

        const int topicMgr;
        const char* topicMgrCat{"TopicManager"};

        const int topic;
        const char* topicCat{"Topic"};

        const int subscriber;
        const char* subscriberCat{"Subscriber"};

        const int election;
        const char* electionCat{"Election"};

        const int replication;
        const char* replicationCat{"Replication"};

        const Ice::LoggerPtr logger;
    };

} // End namespace IceStorm

#endif
