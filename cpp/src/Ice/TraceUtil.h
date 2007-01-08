// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
void traceSlicing(const char*, const ::std::string&, const char *, const ::Ice::LoggerPtr&);

}

#endif
