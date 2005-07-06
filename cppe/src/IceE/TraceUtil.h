// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TRACE_UTIL_H
#define ICEE_TRACE_UTIL_H

#include <IceE/LoggerF.h>
#include <IceE/TraceLevelsF.h>

namespace IceInternal
{

class BasicStream;

void traceHeader(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void traceRequest(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
#ifndef ICEE_NO_BATCH
void traceBatchRequest(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
#endif
void traceReply(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);

}

#endif
