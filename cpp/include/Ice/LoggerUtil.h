// Copyright (c) ZeroC, Inc.

#ifndef ICE_LOGGER_UTIL_H
#define ICE_LOGGER_UTIL_H

#include "CommunicatorF.h"
#include "Config.h"
#include "Exception.h"
#include "Logger.h"
#include "Plugin.h"
#include "Proxy.h"

#include <sstream>
#include <utility>

namespace Ice
{
    /// Base class for logger output utility classes.
    /// @headerfile Ice/Ice.h
    class ICE_API LoggerOutputBase
    {
    public:
        LoggerOutputBase() = default;
        LoggerOutputBase(const LoggerOutputBase&) = delete;

        LoggerOutputBase& operator=(const LoggerOutputBase&) = delete;

        /// Gets the collected output.
        [[nodiscard]] std::string str() const;

        /// @private
        std::ostringstream& _stream(); // For internal use only. Don't use in your code.

    private:
        std::ostringstream _os;
    };

    /// @cond INTERNAL
    ICE_API LoggerOutputBase& loggerInsert(LoggerOutputBase& out, const Ice::Exception& ex);

    template<typename T> struct IsException
    {
        static char testex(Ice::Exception*) noexcept;
        static long testex(...) noexcept;

        static constexpr bool value = sizeof(testex(static_cast<T*>(nullptr))) == sizeof(char);
    };

    template<typename T, bool = false> struct LoggerOutputInserter
    {
        static inline LoggerOutputBase& insert(LoggerOutputBase& out, const T& val)
        {
            out._stream() << val;
            return out;
        }
    };

    // Partial specialization
    template<typename T> struct LoggerOutputInserter<T, true>
    {
        static inline LoggerOutputBase& insert(LoggerOutputBase& out, const T& ex) { return loggerInsert(out, ex); }
    };

    template<typename T> inline LoggerOutputBase& operator<<(LoggerOutputBase& out, const T& val)
    {
        return LoggerOutputInserter<T, IsException<T>::value>::insert(out, val);
    }

    template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
    inline LoggerOutputBase& operator<<(LoggerOutputBase& os, const std::optional<Prx>& p)
    {
        return os << (p ? p->ice_toString() : "");
    }

    inline LoggerOutputBase& operator<<(LoggerOutputBase& os, const ObjectPrx& p) { return os << p.ice_toString(); }

    inline LoggerOutputBase& operator<<(LoggerOutputBase& out, const std::exception& ex)
    {
        out._stream() << ex.what();
        return out;
    }

    ICE_API LoggerOutputBase& operator<<(LoggerOutputBase&, std::ios_base& (*)(std::ios_base&));
    /// @endcond

    /// Collects output and flushes it via a logger method.
    /// @headerfile Ice/Ice.h
    template<class L, class LPtr, void (L::*output)(const std::string&)> class LoggerOutput : public LoggerOutputBase
    {
    public:
        /// Constructs a LoggerOutput object with the given logger.
        /// @param logger The logger or logger-like object to log to.
        LoggerOutput(LPtr logger) : _logger(std::move(logger)) {}

        ~LoggerOutput() { flush(); }

        /// Flushes the colleted output to the logger method.
        void flush()
        {
            std::string s = _stream().str();
            if (!s.empty())
            {
                L& ref = *_logger;
                (ref.*output)(s);
            }
            _stream().str("");
        }

    private:
        LPtr _logger;
    };

    /// Flushes output to Logger::print.
    using Print = LoggerOutput<Logger, LoggerPtr, &Logger::print>;

    /// Flushes output to Logger::warning.
    using Warning = LoggerOutput<Logger, LoggerPtr, &Logger::warning>;

    /// Flushes output to Logger::error.
    using Error = LoggerOutput<Logger, LoggerPtr, &Logger::error>;

    /// Flushes output to Logger::trace.
    /// @headerfile Ice/Ice.h
    class ICE_API Trace : public LoggerOutputBase
    {
    public:
        /// Constructs a Trace object with the given logger and category.
        /// @param logger The logger to log to.
        /// @param category The trace category.
        Trace(LoggerPtr logger, std::string category);

        ~Trace();

        /// Calls Logger::trace with the collected output.
        void flush();

    private:
        LoggerPtr _logger;
        std::string _category;
    };

    /// A special plug-in that installs a logger during a communicator's initialization.
    /// Both initialize and destroy are no-op.
    /// @see Ice::InitializationData
    /// @headerfile Ice/Ice.h
    class ICE_API LoggerPlugin : public Ice::Plugin
    {
    public:
        /// Constructs the plug-in with a target communicator and a logger.
        /// @param communicator The communicator in which to install the logger.
        /// @param logger The logger to install.
        LoggerPlugin(const CommunicatorPtr& communicator, const LoggerPtr& logger);

        /// This method is no-op.
        void initialize() override;

        /// This method is no-op.
        void destroy() override;
    };
}

#endif
