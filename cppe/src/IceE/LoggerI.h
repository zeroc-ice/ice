// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOGGER_I_H
#define ICEE_LOGGER_I_H

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
};

}

#endif
