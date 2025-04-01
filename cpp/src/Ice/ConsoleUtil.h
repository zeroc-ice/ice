// Copyright (c) ZeroC, Inc.

#ifndef ICE_CONSOLE_UTIL_H
#define ICE_CONSOLE_UTIL_H

#include "Ice/Config.h"
#include "Ice/StringConverter.h"

#include <ostream>
#include <sstream>

namespace IceInternal
{
#if defined(_WIN32)

    class ConsoleUtil;
    using ConsoleUtilPtr = std::shared_ptr<ConsoleUtil>;

    class ICE_API ConsoleUtil
    {
    public:
        ConsoleUtil();
        void output(const std::string&) const;
        void error(const std::string&) const;

    private:
        std::string toConsoleEncoding(const std::string&) const;
        Ice::StringConverterPtr _converter;
        Ice::StringConverterPtr _consoleConverter;
    };

    const ICE_API ConsoleUtil& getConsoleUtil();

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

    template<typename T> ConsoleOut& operator<<(ConsoleOut& out, const T& val)
    {
        std::ostringstream s;
        s << val;
        getConsoleUtil().output(s.str());
        return out;
    }

    ICE_API ConsoleOut& endl(ConsoleOut&);
    ICE_API ConsoleOut& flush(ConsoleOut&);

    template<typename T> ConsoleErr& operator<<(ConsoleErr& err, const T& val)
    {
        std::ostringstream s;
        s << val;
        getConsoleUtil().error(s.str());
        return err;
    }

    ICE_API ConsoleErr& endl(ConsoleErr&);
    ICE_API ConsoleErr& flush(ConsoleErr&);

    extern ICE_API ConsoleOut consoleOut;
    extern ICE_API ConsoleErr consoleErr;

#else

    ICE_API extern std::ostream& consoleOut;
    ICE_API extern std::ostream& consoleErr;

#endif
}

#endif
