// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_LOGGER_H
#define ICE_RUBY_LOGGER_H

#include <Config.h>
#include <Ice/Logger.h>

namespace IceRuby
{

//
// LoggerWrapper delegates to a Ruby implementation.
//
class LoggerWrapper : public Ice::Logger
{
public:

    LoggerWrapper(VALUE);

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);

    VALUE getObject();

    void mark();

private:

    VALUE _logger;
};
typedef IceUtil::Handle<LoggerWrapper> LoggerWrapperPtr;

bool initLogger(VALUE);

//
// Create an object that delegates to a C++ implementation.
//
VALUE createLogger(const Ice::LoggerPtr&);

}

#endif
