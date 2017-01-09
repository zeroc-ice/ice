// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/ConsoleUtil.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/Mutex.h>

using namespace IceUtilInternal;
using namespace std;

#if defined(_WIN32) && !defined(ICE_OS_UWP)
namespace
{

IceUtil::Mutex* consoleMutex = 0;
ConsoleUtilPtr consoleUtil = 0;

class Init
{
public:

    Init()
    {
        consoleMutex = new IceUtil::Mutex;
        consoleUtil = ICE_MAKE_SHARED(ConsoleUtil);
    }

    ~Init()
    {
        delete consoleMutex;
        consoleMutex = 0;
    }
};

Init init;

}

const ConsoleUtilPtr&
IceUtilInternal::getConsoleUtil()
{
    return consoleUtil;
}

ConsoleOut IceUtilInternal::consoleOut;
ConsoleErr IceUtilInternal::consoleErr;

ConsoleUtil::ConsoleUtil() :
    _converter(IceUtil::getProcessStringConverter()),
    _consoleConverter(IceUtil::createWindowsStringConverter(GetConsoleOutputCP()))
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
		_consoleConverter->fromUTF8(reinterpret_cast<const IceUtil::Byte* > (u8s.data()),
                                    reinterpret_cast<const IceUtil::Byte*>(u8s.data() + u8s.size()),
                                    consoleString);

        return consoleString;
    }
    catch(const IceUtil::IllegalConversionException&)
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
IceUtilInternal::endl(ConsoleOut& out)
{
    fprintf_s(stdout, "\n");
    fflush(stdout);
    return out;
}

ConsoleOut&
IceUtilInternal::flush(ConsoleOut& out)
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
IceUtilInternal::endl(ConsoleErr& err)
{
    fprintf_s(stderr, "\n");
    fflush(stderr);
    return err;
}

ConsoleErr&
IceUtilInternal::flush(ConsoleErr& err)
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
std::ostream& IceUtilInternal::consoleOut = cout;
std::ostream& IceUtilInternal::consoleErr = cerr;
#endif
