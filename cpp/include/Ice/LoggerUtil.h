// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_LOGGER_UTIL_H
#define ICE_LOGGER_UTIL_H

#include <Ice/Logger.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Plugin.h>
#include <Ice/Exception.h>

namespace Ice
{

/**
 * Base class for logger output utility classes.
 * \headerfile Ice/Ice.h
 */
class ICE_API LoggerOutputBase : private IceUtil::noncopyable
{
public:

    /** Obtains the collected output. */
    std::string str() const;

    /// \cond INTERNAL
    std::ostringstream& _stream(); // For internal use only. Don't use in your code.
    /// \endcond

private:

    std::ostringstream _os;
};

/// \cond INTERNAL
ICE_API LoggerOutputBase& loggerInsert(LoggerOutputBase& out, const IceUtil::Exception& ex);

template<typename T>
struct IsException
{
    static char testex(IceUtil::Exception*);
    static long testex(...);

    static const bool value = sizeof(testex(static_cast<T*>(0))) == sizeof(char);
};

template<typename T, bool = false>
struct LoggerOutputInserter
{
    static inline LoggerOutputBase&
    insert(LoggerOutputBase& out, const T& val)
    {
        out._stream() << val;
        return out;
    }
};

// Partial specialization
template<typename T>
struct LoggerOutputInserter<T, true>
{
    static inline LoggerOutputBase&
    insert(LoggerOutputBase& out, const T& ex)
    {
        return loggerInsert(out, ex);
    }
};

template<typename T>
inline LoggerOutputBase&
operator<<(LoggerOutputBase& out, const T& val)
{
    return LoggerOutputInserter<T, IsException<T>::value>::insert(out, val);
}

#ifdef ICE_CPP11_MAPPING
template<typename T, typename ::std::enable_if<::std::is_base_of<::Ice::ObjectPrx, T>::value>::type* = nullptr>
inline LoggerOutputBase&
operator<<(LoggerOutputBase& os, const ::std::shared_ptr<T>& p)
#else
template<typename T>
inline LoggerOutputBase&
operator<<(LoggerOutputBase& os, const ::IceInternal::ProxyHandle<T>& p)
#endif
{
    return os << (p ? p->ice_toString() : "");
}

inline LoggerOutputBase&
operator<<(LoggerOutputBase& out, const ::std::exception& ex)
{
    out._stream() << ex.what();
    return out;
}

ICE_API LoggerOutputBase& operator<<(LoggerOutputBase&, std::ios_base& (*)(std::ios_base&));
/// \endcond

/**
 * Collects output and flushes it via a logger method.
 * \headerfile Ice/Ice.h
 */
template<class L, class LPtr, void (L::*output)(const std::string&)>
class LoggerOutput : public LoggerOutputBase
{
public:
    inline LoggerOutput(const LPtr& lptr) :
        _logger(lptr)
    {}

    inline ~LoggerOutput()
    {
        flush();
    }

    /** Flushes the colleted output to the logger method. */
    inline void flush()
    {
        std::string s = _stream().str();
        if(!s.empty())
        {
            L& ref = *_logger;
            (ref.*output)(s);
        }
        _stream().str("");
    }

private:

    LPtr _logger;
};

/** Flushes output to Logger::print. */
typedef LoggerOutput<Logger, LoggerPtr, &Logger::print> Print;

/** Flushes output to Logger::warning. */
typedef LoggerOutput<Logger, LoggerPtr, &Logger::warning> Warning;

/** Flushes output to Logger::error. */
typedef LoggerOutput<Logger, LoggerPtr, &Logger::error> Error;

/**
 * Flushes output to Logger::trace.
 * \headerfile Ice/Ice.h
 */
class ICE_API Trace : public LoggerOutputBase
{
public:
    Trace(const LoggerPtr&, const std::string&);
    ~Trace();
    void flush();

private:

    LoggerPtr _logger;
    std::string _category;
};

/**
 * A special plug-in that installs a logger during a communicator's initialization.
 * Both initialize and destroy are no-op. See Ice::InitializationData.
 * \headerfile Ice/Ice.h
 */
class ICE_API LoggerPlugin : public Ice::Plugin
{
public:

    /**
     * Constructs the plug-in with a target communicator and a logger.
     * @param communicator The communicator in which to install the logger.
     * @param logger The logger to be installed.
     */
    LoggerPlugin(const CommunicatorPtr& communicator, const LoggerPtr& logger);

    /** This method is a no-op. */
    virtual void initialize();

    /** This method is a no-op. */
    virtual void destroy();
};

}

#endif
