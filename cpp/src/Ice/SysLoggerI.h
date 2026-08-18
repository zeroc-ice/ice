// Copyright (c) ZeroC, Inc.

#ifndef ICE_SYS_LOGGER_I_H
#define ICE_SYS_LOGGER_I_H

#include "Ice/Logger.h"

#include <string_view>

namespace Ice
{
    class SysLoggerI final : public Logger
    {
    public:
        SysLoggerI(std::string prefix, std::string_view facilityString);
        SysLoggerI(std::string prefix, int facility);
        ~SysLoggerI() override;

        SysLoggerI(const SysLoggerI&) = delete;
        SysLoggerI& operator=(const SysLoggerI&) = delete;

        void print(const std::string&) final;
        void trace(const std::string&, const std::string&) final;
        void warning(const std::string&) final;
        void error(const std::string&) final;
        std::string getPrefix() final;
        LoggerPtr cloneWithPrefix(std::string) final;

    private:
        const int _facility;
        const std::string _prefix;

        // Prepended to each message when this logger's prefix differs from the process-wide syslog ident.
        std::string _bodyPrefix;
    };
}

#endif
