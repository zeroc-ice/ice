// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STORM_TRACE_LEVELS_H
#define ICE_STORM_TRACE_LEVELS_H

#include <IceUtil/Shared.h>
#include <Ice/PropertiesF.h>
#include <Ice/LoggerF.h>

namespace IceStorm
{

class TraceLevels : public ::IceUtil::Shared
{
public:

    TraceLevels(const ::std::string name, const ::Ice::PropertiesPtr&, const Ice::LoggerPtr&);
    virtual ~TraceLevels();

    const int topicMgr;
    const char* topicMgrCat;

    const int topic;
    const char* topicCat;

    const int flush;
    const char* flushCat;

    const int subscriber;
    const char* subscriberCat;

    const Ice::LoggerPtr logger;
};

typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

} // End namespace IceStorm

#endif
