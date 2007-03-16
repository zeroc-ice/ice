// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOGGER_UTIL_H
#define ICEE_LOGGER_UTIL_H

#include <IceE/LoggerF.h>

namespace Ice
{

class ICE_API Print : private IceUtil::noncopyable
{
public:

    Print(const LoggerPtr&);
    ~Print();

    void flush();

    std::string& __str(); // For internal use only. Don't use in your code.

private:

    LoggerPtr _logger;
    std::string _str;
};

inline Print&
operator<<(Print& out, const std::string& val)
{
    out.__str() += val;
    return out;
}

class ICE_API Warning : private IceUtil::noncopyable
{
public:

    Warning(const LoggerPtr&);
    ~Warning();

    void flush();

    std::string& __str(); // For internal use only. Don't use in your code.

private:

    LoggerPtr _logger;
    std::string _str;
};

inline Warning&
operator<<(Warning& out, const std::string& val)
{
    out.__str() += val;
    return out;
}

class ICE_API Error : private IceUtil::noncopyable
{
public:

    Error(const LoggerPtr&);
    ~Error();

    void flush();

    std::string& __str(); // For internal use only. Don't use in your code.

private:

    LoggerPtr _logger;
    std::string _str;
};

inline Error&
operator<<(Error& out, const std::string& val)
{
    out.__str() += val;
    return out;
}

class ICE_API Trace : private IceUtil::noncopyable
{
public:

    Trace(const LoggerPtr&, const std::string&);
    ~Trace();

    void flush();

    std::string& __str(); // For internal use only. Don't use in your code.

private:

    LoggerPtr _logger;
    std::string _category;
    std::string _str;
};

inline Trace&
operator<<(Trace& out, const std::string& val)
{
    out.__str() += val;
    return out;
}

}

#endif
