// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TRACE_UTIL_H
#define ICEE_TRACE_UTIL_H

#include <IceE/TraceLevelsF.h>
#include <IceE/LoggerF.h>

namespace IceInternal
{

class BasicStream;

void traceHeader(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void traceRequest(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
#ifdef ICEE_HAS_BATCH
void traceBatchRequest(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
#endif
void traceReply(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);

}

#endif
