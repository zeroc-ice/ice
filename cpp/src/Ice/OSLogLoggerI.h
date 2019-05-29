//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OSLOG_LOGGER_I_H
#define ICE_OSLOG_LOGGER_I_H

#ifdef __APPLE__

#include <Ice/Logger.h>
#include <Ice/UniqueRef.h>
#include <os/log.h>

namespace Ice
{

class OSLogLoggerI : public Logger
{
public:

    OSLogLoggerI(const std::string&);

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
    virtual std::string getPrefix();
    virtual LoggerPtr cloneWithPrefix(const std::string&);

private:

    const std::string _prefix;
    IceInternal::UniqueRef<os_log_t> _log;
};

}

#endif

#endif
