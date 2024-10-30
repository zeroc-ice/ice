//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OSLOG_LOGGER_I_H
#define ICE_OSLOG_LOGGER_I_H

#ifdef __APPLE__

#    include "Ice/Logger.h"
#    include "UniqueRef.h"
#    include <os/log.h>

namespace Ice
{
    class OSLogLoggerI final : public Logger
    {
    public:
        OSLogLoggerI(std::string);

        void print(const std::string&) final;
        void trace(const std::string&, const std::string&) final;
        void warning(const std::string&) final;
        void error(const std::string&) final;
        std::string getPrefix() final;
        LoggerPtr cloneWithPrefix(std::string) final;

    private:
        const std::string _prefix;
        IceInternal::UniqueRef<os_log_t> _log;
    };
}

#endif

#endif
