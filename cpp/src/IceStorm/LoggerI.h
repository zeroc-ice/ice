// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STORM_LOGGER_I_H
#define ICE_STORM_LOGGER_I_H

#include <Ice/Logger.h>

namespace IceStorm
{

class LoggerI : public Ice::Logger
{
public:

    LoggerI();

    void setPrefix(const std::string&);

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);

private:

    std::string _prefix;
};

typedef IceUtil::Handle<LoggerI> LoggerIPtr;

}

#endif
