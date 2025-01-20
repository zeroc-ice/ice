// Copyright (c) ZeroC, Inc.

#ifdef __APPLE__

#    include "OSLogLoggerI.h"
#    include <os/log.h>

using namespace std;
using namespace Ice;

Ice::OSLogLoggerI::OSLogLoggerI(string prefix)
    : _prefix(std::move(prefix)),
      _subsystem(_prefix.empty() ? "com.zeroc.ice" : "com.zeroc.ice." + _prefix),
      _log(os_log_create(_subsystem.c_str(), ""))
{
}

void
Ice::OSLogLoggerI::print(const string& message)
{
    os_log_with_type(_log.get(), OS_LOG_TYPE_DEFAULT, "%{public}s.", message.c_str());
}

void
Ice::OSLogLoggerI::trace(const string& category, const string& message)
{
    IceInternal::UniqueRef<os_log_t> log(os_log_create(_subsystem.c_str(), category.c_str()));
    os_log_with_type(log.get(), OS_LOG_TYPE_INFO, "%{public}s.", message.c_str());
}

void
Ice::OSLogLoggerI::warning(const string& message)
{
    os_log_with_type(_log.get(), OS_LOG_TYPE_ERROR, "%{public}s.", message.c_str());
}

void
Ice::OSLogLoggerI::error(const string& message)
{
    os_log_with_type(_log.get(), OS_LOG_TYPE_FAULT, "%{public}s.", message.c_str());
}

string
Ice::OSLogLoggerI::getPrefix()
{
    return _prefix;
}

LoggerPtr
Ice::OSLogLoggerI::cloneWithPrefix(string prefix)
{
    return make_shared<OSLogLoggerI>(std::move(prefix));
}

#endif
