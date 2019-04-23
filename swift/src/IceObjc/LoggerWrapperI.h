//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Util.h"

class LoggerWrapperI : public Ice::Logger
{
public:

    LoggerWrapperI(id<ICELoggerProtocol> logger) : _logger(logger)
    {
    }

    virtual ~LoggerWrapperI()
    {
    }

    virtual void
    print(const std::string& msg)
    {
        [_logger print:toNSString(msg)];
    }

    virtual void
    trace(const std::string& category, const std::string& msg)
    {
        [_logger trace:toNSString(category) message:toNSString(msg)];
    }

    virtual void
    warning(const std::string& msg)
    {
        [_logger warning:toNSString(msg)];
    }

    virtual void
    error(const std::string& msg)
    {
        [_logger error:toNSString(msg)];
    }

    virtual std::shared_ptr<Ice::Logger>
    cloneWithPrefix(const std::string& prefix)
    {
        return std::make_shared<LoggerWrapperI>([_logger cloneWithPrefix:toNSString(prefix)]);
    }

    virtual std::string
    getPrefix()
    {
        return fromNSString([_logger getPrefix]);
    }

    id<ICELoggerProtocol>
    getLogger()
    {
        return _logger;
    }

private:

    id<ICELoggerProtocol> _logger;
};
