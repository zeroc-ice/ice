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

namespace Ice
{
    class LoggerI;
    class StderrLoggerI;
}

namespace _Ice
{

void ICE_API _incRef(::Ice::LoggerI*);
void ICE_API _decRef(::Ice::LoggerI*);

void ICE_API _incRef(::Ice::StderrLoggerI*);
void ICE_API _decRef(::Ice::StderrLoggerI*);

}

namespace Ice
{

typedef _Ice::Handle<LoggerI> Logger;
typedef _Ice::Handle<StderrLoggerI> StderrLogger;

}

#endif
