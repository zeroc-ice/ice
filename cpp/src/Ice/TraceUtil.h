// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TRACE_UTIL_H
#define ICE_TRACE_UTIL_H

#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>

namespace IceInternal
{

class BasicStream;

void traceHeader(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void traceRequest(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void traceBatchRequest(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void traceReply(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);

}

#endif
