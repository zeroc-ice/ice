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
#include <Ice/Config.h>

namespace __Ice
{

class Stream;

void traceHeader(const char*, const Stream&,
		 const ::Ice::Logger&, const TraceLevels&);
void traceRequest(const char*, const Stream&,
		  const ::Ice::Logger&, const TraceLevels&);
void traceReply(const char*, const Stream&,
		const ::Ice::Logger&, const TraceLevels&);

}

#endif
