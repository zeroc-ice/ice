// Copyright (c) ZeroC, Inc.
#import "Convert.h"

class LoggerWrapperI final : public Ice::Logger
{
public:
    LoggerWrapperI(id<ICELoggerProtocol> logger) : _logger(logger) {}

    void print(const std::string& msg) final { [_logger print:toNSString(msg)]; }

    void trace(const std::string& category, const std::string& msg) final
    {
        [_logger trace:toNSString(category) message:toNSString(msg)];
    }

    void warning(const std::string& msg) final { [_logger warning:toNSString(msg)]; }

    void error(const std::string& msg) final { [_logger error:toNSString(msg)]; }

    std::shared_ptr<Ice::Logger> cloneWithPrefix(std::string prefix) final
    {
        return std::make_shared<LoggerWrapperI>([_logger cloneWithPrefix:toNSString(prefix)]);
    }

    std::string getPrefix() final { return fromNSString([_logger getPrefix]); }

    id<ICELoggerProtocol> getLogger() { return _logger; }

private:
    id<ICELoggerProtocol> _logger;
};
