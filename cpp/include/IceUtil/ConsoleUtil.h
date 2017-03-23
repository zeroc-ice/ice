// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_CONSOLE_UTIL_H
#define ICE_UTIL_CONSOLE_UTIL_H

#include <IceUtil/Config.h>
#include <IceUtil/StringConverter.h>
#include <iostream>

namespace IceUtilInternal
{

#if defined(_WIN32) && !defined(ICE_OS_UWP)

class ConsoleUtil;
ICE_DEFINE_PTR(ConsoleUtilPtr, ConsoleUtil);

class ICE_API ConsoleUtil
#  ifndef ICE_CPP11_MAPPING
    : public IceUtil::Shared
#  endif
{
public:

    ConsoleUtil();
    void output(const std::string&) const;
    void error(const std::string&) const;

private:

    std::string toConsoleEncoding(const std::string&) const;
    IceUtil::StringConverterPtr _converter;
    IceUtil::StringConverterPtr _consoleConverter;
    static ConsoleUtilPtr _instance;
};

const ICE_API ConsoleUtilPtr& getConsoleUtil();

class ICE_API ConsoleOut
{
public:

    ConsoleOut& operator<<(ConsoleOut& (*pf)(ConsoleOut&));
};

class ICE_API ConsoleErr
{
public:

    ConsoleErr& operator<<(ConsoleErr& (*pf)(ConsoleErr&));
};

template<typename T>
ConsoleOut&
operator<<(ConsoleOut& out, const T& val)
{
    std::ostringstream s;
    s << val;
    getConsoleUtil()->output(s.str());
    return out;
}

ICE_API ConsoleOut& endl(ConsoleOut&);
ICE_API ConsoleOut& flush(ConsoleOut&);
 
template<typename T>
ConsoleErr&
operator<<(ConsoleErr& err, const T& val)
{
    std::ostringstream s;
    s << val;
    getConsoleUtil()->error(s.str());
    return err;
}

ICE_API ConsoleErr& endl(ConsoleErr&);
ICE_API ConsoleErr& flush(ConsoleErr&);

extern ICE_API ConsoleOut consoleOut;
extern ICE_API ConsoleErr consoleErr;

#else

extern ICE_API std::ostream& consoleOut;
extern ICE_API std::ostream& consoleErr;

#endif

}

#endif
