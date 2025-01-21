// Copyright (c) ZeroC, Inc.

#include "ConsoleUtil.h"

#ifdef _WIN32
#    include <mutex>
#    include <windows.h>
#endif

#include <iostream>

using namespace IceInternal;
using namespace std;

#if defined(_WIN32)
namespace
{
    mutex consoleMutex;
    // We leak consoleUtil object to ensure that is available during static destruction.
    ConsoleUtil* consoleUtil = 0;
}

const ConsoleUtil&
IceInternal::getConsoleUtil()
{
    lock_guard sync(consoleMutex);
    if (consoleUtil == 0)
    {
        consoleUtil = new ConsoleUtil();
    }
    return *consoleUtil;
}

ConsoleOut IceInternal::consoleOut;
ConsoleErr IceInternal::consoleErr;

ConsoleUtil::ConsoleUtil()
    : _converter(Ice::getProcessStringConverter()),
      _consoleConverter(Ice::createWindowsStringConverter(GetConsoleOutputCP()))
{
}

string
ConsoleUtil::toConsoleEncoding(const string& message) const
{
    try
    {
        // Convert message to UTF-8
        string u8s = nativeToUTF8(message, _converter);

        // Then from UTF-8 to console CP
        string consoleString;
        _consoleConverter->fromUTF8(
            reinterpret_cast<const byte*>(u8s.data()),
            reinterpret_cast<const byte*>(u8s.data() + u8s.size()),
            consoleString);

        return consoleString;
    }
    catch (const Ice::IllegalConversionException&)
    {
        //
        // If there is a problem with the encoding conversions we just
        // return the original message without encoding conversions.
        //
        return message;
    }
}

void
ConsoleUtil::output(const string& message) const
{
    //
    // Use fprintf_s to avoid encoding conversion when stderr is connected
    // to Windows console.
    //
    fprintf_s(stdout, "%s", toConsoleEncoding(message).c_str());
}

void
ConsoleUtil::error(const string& message) const
{
    //
    // Use fprintf_s to avoid encoding conversion when stderr is connected
    // to Windows console.
    //
    fprintf_s(stderr, "%s", toConsoleEncoding(message).c_str());
}

ConsoleOut&
IceInternal::endl(ConsoleOut& out)
{
    fprintf_s(stdout, "\n");
    fflush(stdout);
    return out;
}

ConsoleOut&
IceInternal::flush(ConsoleOut& out)
{
    fflush(stdout);
    return out;
}

ConsoleOut&
ConsoleOut::operator<<(ConsoleOut& (*pf)(ConsoleOut&))
{
    pf(*this);
    return *this;
}

ConsoleErr&
IceInternal::endl(ConsoleErr& err)
{
    fprintf_s(stderr, "\n");
    fflush(stderr);
    return err;
}

ConsoleErr&
IceInternal::flush(ConsoleErr& err)
{
    fflush(stderr);
    return err;
}

ConsoleErr&
ConsoleErr::operator<<(ConsoleErr& (*pf)(ConsoleErr&))
{
    pf(*this);
    return *this;
}
#else
std::ostream& IceInternal::consoleOut = cout;
std::ostream& IceInternal::consoleErr = cerr;
#endif
