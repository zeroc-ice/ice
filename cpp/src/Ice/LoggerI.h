// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOGGER_I_H
#define ICE_LOGGER_I_H

#include <Ice/Logger.h>
#include <IceUtil/FileUtil.h>

namespace Ice
{

class LoggerI : public Logger
{
public:

    LoggerI(const std::string&, const std::string&);
    ~LoggerI();

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
    virtual LoggerPtr cloneWithPrefix(const std::string&);

private:

    void write(const std::string&, bool);

    std::string _prefix;
    IceUtilInternal::ofstream _out;
    std::string _file;
};

typedef IceUtil::Handle<LoggerI> LoggerIPtr;

}

#endif
