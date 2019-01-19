//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_TRACE_UTIL_H
#define ICE_TRACE_UTIL_H

#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>

namespace Ice
{

class OutputStream;
class InputStream;

}

namespace IceInternal
{

void traceSend(const ::Ice::OutputStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void traceRecv(const ::Ice::InputStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void trace(const char*, const ::Ice::OutputStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void trace(const char*, const ::Ice::InputStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void traceSlicing(const char*, const ::std::string&, const char *, const ::Ice::LoggerPtr&);

}

#endif
