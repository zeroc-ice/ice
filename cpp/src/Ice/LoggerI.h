// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOGGER_I_H
#define ICE_LOGGER_I_H

#include <IceUtil/Mutex.h>
#include <Ice/Logger.h>

namespace Ice
{

class LoggerI : public Logger, public ::IceUtil::Mutex
{
public:

    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
};

}

#endif
