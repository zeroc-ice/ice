// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOGGER_I_H
#define ICEE_LOGGER_I_H

#include <IceE/Mutex.h>
#include <IceE/Logger.h>

namespace IceE
{

class LoggerI : public Logger
{
public:

    LoggerI(const std::string&, bool);

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);

private:

    std::string _prefix;
    std::string _emptyPrefix;
    bool _timestamp;

    //
    // A global mutex is used to avoid garbled output with multiple
    // communicators.
    //
    static IceE::Mutex _globalMutex;
};

}

#endif
