// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOGGER_H
#define ICE_LOGGER_H

#include <Ice/LoggerF.h>
#include <Ice/InstanceF.h>
#include <Ice/Shared.h>

namespace IceLocal
{

class ICE_API LoggerI : public ::_Ice::Shared
{
public:
    
    LoggerI();
    virtual ~LoggerI();

    virtual void trace(const std::string&, const std::string&) = 0;

private:

    LoggerI(const LoggerI&);
    void operator=(const LoggerI&);
};

class ICE_API StderrLoggerI : public LoggerI, public JTCMutex
{
public:

    StderrLoggerI();

    virtual void trace(const std::string&, const std::string&);

private:

    StderrLoggerI(const StderrLoggerI&);
    void operator=(const StderrLoggerI&);
};

}

#endif
