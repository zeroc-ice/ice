// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_SYS_LOGGER_I_H
#define ICE_SYS_LOGGER_I_H

#include <IceUtil/Mutex.h>
#include <Ice/Logger.h>

namespace Ice
{

class SysLoggerI : public Logger, public ::IceUtil::Mutex
{
public:

    SysLoggerI(const std::string&, const std::string&);
    SysLoggerI(const std::string&, int);
    ~SysLoggerI();

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
    virtual std::string getPrefix();
    virtual LoggerPtr cloneWithPrefix(const std::string&);

private:

    int _facility;
    const std::string _prefix;
};

}

#endif
