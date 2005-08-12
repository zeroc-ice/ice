// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOGGER_I_H
#define ICE_LOGGER_I_H

#include <IceE/Logger.h>
#include <IceE/Mutex.h>

namespace Ice
{

class LoggerI : public Logger
{
public:

    LoggerI(const std::string&);

    virtual void print(const std::string&);
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
