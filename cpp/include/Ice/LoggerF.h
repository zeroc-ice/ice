// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOGGER_F_H
#define ICE_LOGGER_F_H

#include <Ice/Handle.h>

namespace IceLocal
{
    class LoggerI;
    class StderrLoggerI;
}

namespace _Ice
{

void ICE_API _incRef(::IceLocal::LoggerI*);
void ICE_API _decRef(::IceLocal::LoggerI*);

void ICE_API _incRef(::IceLocal::StderrLoggerI*);
void ICE_API _decRef(::IceLocal::StderrLoggerI*);

}

namespace IceLocal
{

typedef _Ice::Handle<LoggerI> Logger;
typedef _Ice::Handle<StderrLoggerI> StderrLogger;

}

#endif
