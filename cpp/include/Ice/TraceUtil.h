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

#include <Ice/Config.h>

namespace __Ice
{

class Stream;

void traceHeader(const char*, const Stream&);
void traceRequest(const char*, const Stream&);
void traceReply(const char*, const Stream&);

}

#endif
