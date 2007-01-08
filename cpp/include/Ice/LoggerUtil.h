// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOGGER_UTIL_H
#define ICE_LOGGER_UTIL_H

#include <Ice/LoggerF.h>

namespace Ice
{

class ICE_API Print : private IceUtil::noncopyable
{
public:

    Print(const LoggerPtr&);
    ~Print();

    void flush();

    std::ostringstream& __str(); // For internal use only. Don't use in your code.

private:

    LoggerPtr _logger;
    std::ostringstream _str;
};

template<typename T>
inline Print&
operator<<(Print& out, const T& val)
{
    out.__str() << val;
    return out;
}

template<class Y>
::Ice::Print& operator<<(::Ice::Print& os, ::IceInternal::ProxyHandle<Y> p)
{
    return os << (p ? p->ice_toString() : "");
}

ICE_API Print& operator<<(Print&, std::ios_base& (*)(std::ios_base&));

class ICE_API Warning : private IceUtil::noncopyable
{
public:

    Warning(const LoggerPtr&);
    ~Warning();

    void flush();

    std::ostringstream& __str(); // For internal use only. Don't use in your code.

private:

    LoggerPtr _logger;
    std::ostringstream _str;
};

template<typename T>
inline Warning&
operator<<(Warning& out, const T& val)
{
    out.__str() << val;
    return out;
}

template<class Y>
::Ice::Warning& operator<<(::Ice::Warning& os, ::IceInternal::ProxyHandle<Y> p)
{
    return os << (p ? p->ice_toString() : "");
}

ICE_API Warning& operator<<(Warning&, std::ios_base& (*)(std::ios_base&));

class ICE_API Error : private IceUtil::noncopyable
{
public:

    Error(const LoggerPtr&);
    ~Error();

    void flush();

    std::ostringstream& __str(); // For internal use only. Don't use in your code.

private:

    LoggerPtr _logger;
    std::ostringstream _str;
};

template<typename T>
inline Error&
operator<<(Error& out, const T& val)
{
    out.__str() << val;
    return out;
}

template<class Y>
::Ice::Error& operator<<(::Ice::Error& os, ::IceInternal::ProxyHandle<Y> p)
{
    return os << (p ? p->ice_toString() : "");
}
ICE_API Error& operator<<(Error&, std::ios_base& (*)(std::ios_base&));

class ICE_API Trace : private IceUtil::noncopyable
{
public:

    Trace(const LoggerPtr&, const std::string&);
    ~Trace();

    void flush();

    std::ostringstream& __str(); // For internal use only. Don't use in your code.

private:

    LoggerPtr _logger;
    std::string _category;
    std::ostringstream _str;
};

template<typename T>
inline Trace&
operator<<(Trace& out, const T& val)
{
    out.__str() << val;
    return out;
}

template<class Y>
::Ice::Trace& operator<<(::Ice::Trace& os, ::IceInternal::ProxyHandle<Y> p)
{
    return os << (p ? p->ice_toString() : "");
}

ICE_API Trace& operator<<(Trace&, std::ios_base& (*)(std::ios_base&));

}

#endif
