// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

class ICE_API LoggerOutputBase : private IceUtil::noncopyable
{
public:

    std::string str() const;
   
    std::ostringstream& __str(); // For internal use only. Don't use in your code.

private:

    std::ostringstream _str;
};

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
        out.__str() << val;
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

template<typename T>
inline LoggerOutputBase& 
operator<<(LoggerOutputBase& os, const ::IceInternal::ProxyHandle<T>& p)
{
    return os << (p ? p->ice_toString() : "");
}

inline LoggerOutputBase&
operator<<(LoggerOutputBase& out, const ::std::exception& ex)
{
    out.__str() << ex.what();
    return out;
}

ICE_API LoggerOutputBase& operator<<(LoggerOutputBase&, std::ios_base& (*)(std::ios_base&));

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

    inline void flush()
    {
        std::string s = __str().str();
        if(!s.empty())
        {
            L& ref = *_logger;
            (ref.*output)(s);
        }
        __str().str("");
    }

private:
   
    LPtr _logger;
};

typedef LoggerOutput<Logger, LoggerPtr, &Logger::print> Print;
typedef LoggerOutput<Logger, LoggerPtr, &Logger::warning> Warning;
typedef LoggerOutput<Logger, LoggerPtr, &Logger::error> Error;

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

//
// A special plug-in that installs a logger during a communicator's initialization.
// Both initialize and destroy are no-op. See Ice::InitializationData.
//
class ICE_API LoggerPlugin : public Ice::Plugin
{
public:

    LoggerPlugin(const CommunicatorPtr& communicator, const LoggerPtr&);

    virtual void initialize();

    virtual void destroy();
};

}

#endif
