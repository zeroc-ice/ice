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

namespace Ice
{

class ICE_API Logger : public ::__Ice::Shared
{
public:
    
    Logger() { }
    virtual void trace(const std::string&, const std::string&) = 0;
    virtual void warning(const std::string&) = 0;
    virtual void error(const std::string&) = 0;
};

class ICE_API StderrLogger : public Logger, public JTCMutex
{
public:

    StderrLogger();
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
};

}

#endif
