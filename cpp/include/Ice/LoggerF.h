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

namespace __Ice
{

void ICE_API incRef(::Ice::LoggerI*);
void ICE_API decRef(::Ice::LoggerI*);

void ICE_API incRef(::Ice::StderrLoggerI*);
void ICE_API decRef(::Ice::StderrLoggerI*);

}

namespace Ice
{

typedef __Ice::Handle<LoggerI> Logger;
typedef __Ice::Handle<StderrLoggerI> StderrLogger;

}

#endif
