// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_LOGGER_I_H
#define ICE_LOGGER_I_H

#include <IceUtil/Mutex.h>
#include <Ice/Logger.h>

namespace Ice
{

class LoggerI : public Logger
{
public:
    LoggerI(const std::string&);

    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);

private:

    std::string _prefix;
    std::string _emptyPrefix;

    //
    // A global mutex is used to avoid garbled output with multiple
    // communicators.
    //
    static IceUtil::Mutex _globalMutex;
};

}

#endif
