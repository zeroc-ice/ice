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
    class Logger;
    class StderrLogger;
}

namespace __Ice
{

void ICE_API incRef(::Ice::Logger*);
void ICE_API decRef(::Ice::Logger*);

void ICE_API incRef(::Ice::StderrLogger*);
void ICE_API decRef(::Ice::StderrLogger*);

}

namespace Ice
{

typedef __Ice::Handle<Logger> Logger_ptr;
typedef __Ice::Handle<StderrLogger> StderrLogger_ptr;

}

#endif
