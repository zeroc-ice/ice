//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifdef __APPLE__

#include <Ice/OSLogLoggerI.h>
#include <os/log.h>

using namespace std;
using namespace Ice;

Ice::OSLogLoggerI::OSLogLoggerI(const std::string& prefix) : _prefix(prefix)
{
    const string subsystem = prefix.empty() ? "com.zeroc.ice" : "com.zeroc.ice." + prefix;
    _log.reset(os_log_create(subsystem.c_str(), ""));
}

void
Ice::OSLogLoggerI::print(const std::string& message)
{
    os_log_with_type(_log.get(), OS_LOG_TYPE_DEFAULT, "%{public}s.", message.c_str());
}

void
Ice::OSLogLoggerI::trace(const std::string& category, const std::string& message)
{
    const string subsystem = _prefix.empty() ? "com.zeroc.ice" : "com.zeroc.ice." + _prefix;
    IceInternal::UniqueRef<os_log_t> log(os_log_create(subsystem.c_str(), category.c_str()));
    os_log_with_type(log.get(), OS_LOG_TYPE_INFO, "%{public}s.", message.c_str());
}

void
Ice::OSLogLoggerI::warning(const std::string& message)
{
    os_log_with_type(_log.get(), OS_LOG_TYPE_ERROR, "%{public}s.", message.c_str());
}

void
Ice::OSLogLoggerI::error(const std::string& message)
{
    os_log_with_type(_log.get(), OS_LOG_TYPE_FAULT, "%{public}s.", message.c_str());
}

std::string
Ice::OSLogLoggerI::getPrefix()
{
    return _prefix;
}

LoggerPtr
Ice::OSLogLoggerI::cloneWithPrefix(const std::string& prefix)
{
    return ICE_MAKE_SHARED(OSLogLoggerI, prefix);
}

#endif
