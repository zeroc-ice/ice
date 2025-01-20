// Copyright (c) ZeroC, Inc.

#ifndef ICE_SYS_LOGGER_I_H
#define ICE_SYS_LOGGER_I_H

#include "Ice/Logger.h"

#include <mutex>
#include <string_view>

namespace Ice
{
    class SysLoggerI final : public Logger
    {
    public:
        SysLoggerI(std::string prefix, std::string_view facilityString);
        SysLoggerI(std::string prefix, int facility);
        ~SysLoggerI() override;

        void print(const std::string&) final;
        void trace(const std::string&, const std::string&) final;
        void warning(const std::string&) final;
        void error(const std::string&) final;
        std::string getPrefix() final;
        LoggerPtr cloneWithPrefix(std::string) final;

    private:
        int _facility;
        const std::string _prefix;
        std::mutex _mutex;
    };
}

#endif
