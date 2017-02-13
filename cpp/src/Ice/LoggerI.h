// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOGGER_I_H
#define ICE_LOGGER_I_H

#include <Ice/Logger.h>
#include <Ice/StringConverter.h>
#include <fstream>

namespace Ice
{

class LoggerI : public Logger
{
public:

    LoggerI(const std::string&, const std::string&, bool convert = true, std::size_t sizeMax = 0);
    ~LoggerI();

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
    virtual std::string getPrefix();
    virtual LoggerPtr cloneWithPrefix(const std::string&);

private:

    void write(const std::string&, bool);

    const std::string _prefix;
    std::string _formattedPrefix;
    const bool _convert;
    const StringConverterPtr _converter;
    std::ofstream _out;

    std::string _file;
    std::size_t _sizeMax;

    //
    // In case of a log file rename failure is set to the time in milliseconds
    // after which rename could be attempted again. Otherwise is set to zero.
    //
    IceUtil::Time _nextRetry;
};
ICE_DEFINE_PTR(LoggerIPtr, LoggerI);

}

#endif
