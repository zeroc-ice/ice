// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
