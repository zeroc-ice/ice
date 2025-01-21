// Copyright (c) ZeroC, Inc.

#ifndef ICE_LOGGER_I_H
#define ICE_LOGGER_I_H

#include "Ice/Logger.h"
#include "Ice/StringConverter.h"
#include <chrono>
#include <fstream>

namespace Ice
{
    class LoggerI final : public Logger
    {
    public:
        LoggerI(std::string prefix, std::string file, bool convert = true, std::size_t sizeMax = 0);
        ~LoggerI() override;

        void print(const std::string&) final;
        void trace(const std::string& category, const std::string& message) final;
        void warning(const std::string&) final;
        void error(const std::string&) final;
        std::string getPrefix() final;
        LoggerPtr cloneWithPrefix(std::string) final;

    private:
        void write(const std::string&, bool);

        const std::string _prefix;
        std::string _formattedPrefix;
        const bool _convert;
        const StringConverterPtr _converter;
        std::ofstream _out;

        const std::string _file;
        std::size_t _sizeMax;

        //
        // In case of a log file rename failure is set to the time in milliseconds
        // after which rename could be attempted again. Otherwise is set to zero.
        //
        std::chrono::steady_clock::time_point _nextRetry;
    };
    using LoggerIPtr = std::shared_ptr<LoggerI>;
}

#endif
