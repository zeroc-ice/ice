//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#if defined(_MSC_VER)
//
// DbgHelp.dll on Windows XP does not contain Unicode functions, so we
// "switch on" Unicode only with VS2012 and up
//
#  ifndef UNICODE
#    define UNICODE
#  endif
#  ifndef _UNICODE
#    define _UNICODE
#  endif
#endif

//
// For UINTPTR_MAX on Ubuntu Precise
//
#ifndef __STDC_LIMIT_MACROS
#   define __STDC_LIMIT_MACROS
#endif

#include <IceUtil/Exception.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/StringUtil.h>
#include <ostream>
#include <iomanip>
#include <cstdlib>

#ifdef __GNUC__
#   if defined(ICE_LIBBACKTRACE)
#       include <backtrace.h>
#       include <backtrace-supported.h>
#       if BACKTRACE_SUPPORTED && BACKTRACE_SUPPORTS_THREADS
#           include <algorithm>
#           include <cxxabi.h>
#       else
            // It's available but we cant' use it - shouldn't happen
#           undef ICE_LIBBACKTRACE
#       endif
#   endif

#   if !defined(_AIX) && !defined(__sun) && !defined(__FreeBSD__) && !defined(ICE_STATIC_LIBS)
#       include <execinfo.h>
#       include <cxxabi.h>
#       include <stdint.h>
#       define ICE_BACKTRACE
#   endif
#endif

//
// The Slice compilers don't retrieve the exception stack traces so we don't need the DbgHelp calls.
//
#if defined(_WIN32) && !defined(ICE_BUILDING_SLICE_COMPILERS)
#   define ICE_DBGHELP
#   define DBGHELP_TRANSLATE_TCHAR
#   include <IceUtil/StringConverter.h>
#   pragma warning(disable:4091) // VS 2015 RC issues this warning for code in DbgHelp.h
#   include <DbgHelp.h>
#   include <tchar.h>
#endif

using namespace std;

namespace IceUtilInternal
{

#ifdef NDEBUG
bool ICE_API printStackTraces = false;
#else
bool ICE_API printStackTraces = true;
#endif

bool ICE_API nullHandleAbort = false;

StackTraceImpl
stackTraceImpl()
{
#if defined(ICE_DBGHELP)
    return STDbghelp;
#elif defined(ICE_LIBBACKTRACE)
#   if defined(ICE_BACKTRACE)
    return STLibbacktracePlus;
#   else
    return STLibbacktrace;
#   endif
#elif defined(ICE_BACKTRACE)
    return STBacktrace;
#else
    return STNone;
#endif
}
}

namespace
{

IceUtil::Mutex* globalMutex = 0;

#ifdef ICE_DBGHELP
HANDLE process = 0;
#endif

#ifdef ICE_LIBBACKTRACE
backtrace_state* bstate = 0;

void
ignoreErrorCallback(void*, const char* /*msg*/, int /*errnum*/)
{
    // cerr << "Error callback: " << msg << ", errnum = " << errnum << endl;
}

int
ignoreFrame(void*, ICE_MAYBE_UNUSED uintptr_t pc, const char*, int, const char*)
{
    assert(pc == 0);
    return 0;
}

#endif

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
#ifdef ICE_LIBBACKTRACE
        // Leaked, as libbacktrace does not provide an API to free this state.
        //
        bstate = backtrace_create_state(0, 1, ignoreErrorCallback, 0);

        // The first call to backtrace_pcinfo does not initialize bstate->fileline_fn
        // in a thread-safe manner, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81098.
        // So we make a "dummy" call to backtrace_pcinfo to initialize it here.
        //
        backtrace_pcinfo(bstate, 0, ignoreFrame, ignoreErrorCallback, 0);
#endif
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
#ifdef ICE_DBGHELP
        if(process != 0)
        {
            SymCleanup(process);
            process = 0;
        }
#endif
    }
};

Init init;

#if defined(ICE_LIBBACKTRACE) || defined (ICE_BACKTRACE)

struct FrameInfo
{
    FrameInfo(int i, uintptr_t p) :
        index(i),
        pc(p),
        fallback(0),
        setByErrorCb(false)
    {
    }

    int index;
    uintptr_t pc;
    const char* fallback;
    bool setByErrorCb;
    string output;
};

void
decode(const string& line, string& function, string& filename)
{
    string::size_type openParen = line.find_first_of('(');
    if(openParen != string::npos)
    {
        //
        // Format: "/opt/Ice/lib/libIceUtil.so.33(_ZN7IceUtil9ExceptionC2EPKci+0x51) [0x73b267]"
        //
        string::size_type closeParen = line.find_first_of(')', openParen);
        if(closeParen != string::npos)
        {
            string tmp = line.substr(openParen + 1, closeParen - openParen - 1);
            string::size_type plus = tmp.find_last_of('+');
            if(plus != string::npos)
            {
                function = tmp.substr(0 , plus);
                filename = line.substr(0, openParen);
            }
        }
    }
    else
    {
        //
        // Format: "1    libIce.3.3.1.dylib   0x000933a1 _ZN7IceUtil9ExceptionC2EPKci + 71"
        //
        string::size_type plus = line.find_last_of('+');
        if(plus != string::npos)
        {
            string tmp = line.substr(0, plus - 1);
            string::size_type space = tmp.find_last_of(" \t");
            if(space != string::npos)
            {
                tmp = tmp.substr(space + 1, tmp.size() - space);

                string::size_type start = line.find_first_not_of(" \t", 3);
                if(start != string::npos)
                {
                    string::size_type finish = line.find_first_of(" \t", start);
                    if(finish != string::npos)
                    {
                        function = tmp;
                        filename = line.substr(start, finish - start);
                    }
                }
            }
        }
    }
}

int
printFrame(void* data, uintptr_t pc, const char* filename, int lineno, const char* function)
{
    FrameInfo& frameInfo = *reinterpret_cast<FrameInfo*>(data);

    ostringstream os;
    os << setw(3) << frameInfo.index << " ";

    string functionHolder, filenameHolder;

    if(!function && frameInfo.fallback)
    {
        // Extract function and filename from fallback
        decode(frameInfo.fallback, functionHolder, filenameHolder);
        if(!functionHolder.empty())
        {
            function = functionHolder.c_str();
        }
        if(!filename && !filenameHolder.empty())
        {
            filename = filenameHolder.c_str();
        }
    }

    int ret = 0;

    if(function)
    {
        char* demangledFunction = abi::__cxa_demangle(function, 0, 0, 0);
        if(demangledFunction)
        {
            os << demangledFunction;
            free(demangledFunction);
        }
        else
        {
            os << function;
        }

        if(filename && lineno > 0)
        {
            os << " at " << filename << ":" << lineno;
        }
        else if(filename)
        {
            os << " in " << filename;
        }
    }
    else if(frameInfo.fallback)
    {
        // decode was not able to parse this string
        os << frameInfo.fallback;
        ret = 1;
    }
    else
    {
        os << hex << setw(sizeof(uintptr_t) * 2) << setfill('0') << pc;
        ret = 2;
    }
    os << "\n";
    frameInfo.output = os.str();
    return ret;
}
#endif

#ifdef ICE_LIBBACKTRACE

void
handlePcInfoError(void* data, const char* /*msg*/, int /*errnum*/)
{
    FrameInfo& frameInfo = *reinterpret_cast<FrameInfo*>(data);
    printFrame(&frameInfo, frameInfo.pc, 0, 0, 0);
    frameInfo.setByErrorCb = true;
}

int
addFrame(void* sf, uintptr_t pc)
{
    if(pc != UINTPTR_MAX)
    {
        vector<void*>* stackFrames = reinterpret_cast<vector<void*>*>(sf);
        stackFrames->push_back(reinterpret_cast<void*>(pc));
        return 0;
    }
    else
    {
        return 1;
    }
}
#endif

vector<void*>
getStackFrames()
{
    vector<void*> stackFrames;

    if(!IceUtilInternal::printStackTraces)
    {
        return stackFrames;
    }

#if defined(ICE_DBGHELP)

    stackFrames.resize(61);
    //
    // 1: skip the first frame (the call to getStackFrames)
    // 1 + stackSize < 63 on Windows XP according to the documentation for CaptureStackBackTrace
    //
    USHORT frameCount = CaptureStackBackTrace(1, static_cast<DWORD>(stackFrames.size()), &stackFrames.front(), 0);

    stackFrames.resize(frameCount);

#elif defined(ICE_LIBBACKTRACE)

    backtrace_simple(bstate, 1, addFrame, ignoreErrorCallback, &stackFrames);

#elif defined(ICE_BACKTRACE)

    stackFrames.resize(100);
    int stackDepth = backtrace(&stackFrames.front(), static_cast<int>(stackFrames.size()));
    stackFrames.resize(static_cast<size_t>(stackDepth));
    if(!stackFrames.empty())
    {
        stackFrames.erase(stackFrames.begin()); // drop the first frame
    }
#endif

    return stackFrames;
}

string
getStackTrace(const vector<void*>& stackFrames)
{
    if(stackFrames.empty())
    {
        return "";
    }

    string stackTrace;

#if defined(ICE_DBGHELP)

    //
    // Note: the Sym functions are not thread-safe
    //
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);
    bool refreshModuleList = process != 0;
    if(process == 0)
    {
        process = GetCurrentProcess();

        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS | SYMOPT_EXACT_SYMBOLS | SYMOPT_UNDNAME);
        if(SymInitialize(process, 0, TRUE) == 0)
        {
            process = 0;
            return "No stack trace: SymInitialize failed with " + IceUtilInternal::errorToString(GetLastError());
        }
    }
    lock.release();

#if defined(_MSC_VER)
#   if defined(DBGHELP_TRANSLATE_TCHAR)
    static_assert(sizeof(TCHAR) == sizeof(wchar_t), "Bad TCHAR - should be wchar_t");
#   else
    static_assert(sizeof(TCHAR) == sizeof(char), "Bad TCHAR - should be char");
#   endif
#endif

    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];

    SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_SYM_NAME;

    IMAGEHLP_LINE64 line = {};
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    DWORD displacement = 0;

    lock.acquire();

    if(refreshModuleList && SymRefreshModuleList(process) == 0)
    {
        return "No stack trace: SymRefreshModuleList failed with " + IceUtilInternal::errorToString(GetLastError());
    }
#ifdef DBGHELP_TRANSLATE_TCHAR
    const IceUtil::StringConverterPtr converter = IceUtil::getProcessStringConverter();
#endif
    for(size_t i = 0; i < stackFrames.size(); i++)
    {
        ostringstream s;
        s << setw(3) << i << " ";

        DWORD64 address = reinterpret_cast<DWORD64>(stackFrames[i]);

        BOOL ok = SymFromAddr(process, address, 0, symbol);
        if(ok)
        {
#ifdef DBGHELP_TRANSLATE_TCHAR
            s << wstringToString(symbol->Name, converter);
#else
            s << symbol->Name;
#endif
            ok = SymGetLineFromAddr64(process, address, &displacement, &line);
            if(ok)
            {
                s << " at "
#ifdef DBGHELP_TRANSLATE_TCHAR
                  << wstringToString(line.FileName, converter)
#else
                  << line.FileName
#endif
                  << ":" << line.LineNumber;
            }
        }
        else
        {
            s << hex << setw(sizeof(DWORD64) * 2) << setfill('0') << address;
        }
        s << "\n";
        stackTrace += s.str();
    }
    lock.release();

#elif defined(ICE_LIBBACKTRACE) || defined (ICE_BACKTRACE)

    vector<void*>::const_iterator p = stackFrames.begin();
    int frameIndex = 0;
    int offset = 0;
    char** backtraceStrings = 0;

#   if defined(ICE_LIBBACKTRACE) && defined(ICE_BACKTRACE)
    bool backtraceStringsInitialized = false;
#   endif
#   if !defined(ICE_LIBBACKTRACE)
    // Initialize backtraceStrings immediately
    if(p != stackFrames.end())
    {
        backtraceStrings = backtrace_symbols(&*p, static_cast<int>(stackFrames.size()));
    }
#   endif

    do
    {
        FrameInfo frameInfo(frameIndex, reinterpret_cast<uintptr_t>(*p));
        bool retry = false;

        if(backtraceStrings)
        {
            frameInfo.fallback = backtraceStrings[frameIndex - offset];
        }

#   if defined(ICE_LIBBACKTRACE)
        bool ok = backtrace_pcinfo(bstate, frameInfo.pc, printFrame, handlePcInfoError, &frameInfo) == 0;

        // When error callback is called, pcinfo returns 0
        if(!ok || frameInfo.setByErrorCb)
        {
#       if defined(ICE_BACKTRACE)
            if(!backtraceStringsInitialized)
            {
                offset = frameIndex;
                // Initialize backtraceStrings as fallback
                backtraceStrings = backtrace_symbols(&*p, stackFrames.size() - offset);
                backtraceStringsInitialized = true;
                retry = true;
            }
#       endif
        }
#   else // not using libbacktrace:
        printFrame(&frameInfo, frameInfo.pc, 0, 0, 0);
#   endif
        if(!retry)
        {
            stackTrace += frameInfo.output;
            ++p;
            ++frameIndex;
        }
    } while(p != stackFrames.end());

    if(backtraceStrings)
    {
        free(backtraceStrings);
    }

#endif
    return stackTrace;
}
}

IceUtil::Exception::Exception() :
    _file(0),
    _line(0),
    _stackFrames(getStackFrames())
{
}

IceUtil::Exception::Exception(const char* file, int line) :
    _file(file),
    _line(line),
    _stackFrames(getStackFrames())
{
}

void
IceUtil::Exception::ice_print(ostream& out) const
{
    if(_file && _line > 0)
    {
        out << _file << ':' << _line << ": ";
    }
    out << ice_id();
}

const char*
IceUtil::Exception::what() const noexcept
{
    try
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);
        {
            if(_str.empty())
            {
                stringstream s;
                ice_print(s);
                _str = s.str(); // Lazy initialization.
            }
        }
        return _str.c_str();
    }
    catch(...)
    {
    }
    return "";
}

const char*
IceUtil::Exception::ice_file() const
{
    return _file;
}

int
IceUtil::Exception::ice_line() const
{
    return _line;
}

string
IceUtil::Exception::ice_stackTrace() const
{
    return getStackTrace(_stackFrames);
}

unique_ptr<IceUtil::Exception>
IceUtil::Exception::ice_clone() const
{
    return unique_ptr<Exception>(ice_cloneImpl());
}

ostream&
IceUtil::operator<<(ostream& out, const IceUtil::Exception& ex)
{
    ex.ice_print(out);
    return out;
}

IceUtil::NullHandleException::NullHandleException(const char* file, int line) :
    ExceptionHelper<NullHandleException>(file, line)
{
    if(IceUtilInternal::nullHandleAbort)
    {
        abort();
    }
}

string
IceUtil::NullHandleException::ice_id() const
{
    return "::IceUtil::NullHandleException";
}

IceUtil::IllegalArgumentException::IllegalArgumentException(const char* file, int line) :
    ExceptionHelper<IllegalArgumentException>(file, line)
{
}

IceUtil::IllegalArgumentException::IllegalArgumentException(const char* file, int line, const string& r) :
    ExceptionHelper<IllegalArgumentException>(file, line),
    _reason(r)
{
}

void
IceUtil::IllegalArgumentException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ": " << _reason;
}

string
IceUtil::IllegalArgumentException::ice_id() const
{
    return "::IceUtil::IllegalArgumentException";
}

string
IceUtil::IllegalArgumentException::reason() const
{
    return _reason;
}

//
// IllegalConversionException
//
IceUtil::IllegalConversionException::IllegalConversionException(const char* file, int line):
    ExceptionHelper<IllegalConversionException>(file, line)
{
}

IceUtil::IllegalConversionException::IllegalConversionException(const char* file, int line,
                                                                const string& reason):
    ExceptionHelper<IllegalConversionException>(file, line),
    _reason(reason)
{
}

void
IceUtil::IllegalConversionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ": " << _reason;

}

string
IceUtil::IllegalConversionException::ice_id() const
{
    return "::IceUtil::IllegalConversionException";
}

string
IceUtil::IllegalConversionException::reason() const
{
    return _reason;
}

IceUtil::SyscallException::SyscallException(const char* file, int line, int err ):
    ExceptionHelper<SyscallException>(file, line),
    _error(err)
{
}

void
IceUtil::SyscallException::ice_print(ostream& os) const
{
    Exception::ice_print(os);
    if(_error != 0)
    {
        os << ":\nsyscall exception: " << IceUtilInternal::errorToString(_error);
    }
}

string
IceUtil::SyscallException::ice_id() const
{
    return "::IceUtil::SyscallException";
}

int
IceUtil::SyscallException::error() const
{
    return _error;
}

IceUtil::FileLockException::FileLockException(const char* file, int line, int err, const string& path):
    ExceptionHelper<FileLockException>(file, line),
    _error(err),
    _path(path)
{
}

void
IceUtil::FileLockException::ice_print(ostream& os) const
{
    Exception::ice_print(os);
    os << ":\ncould not lock file: `" << _path << "'";
    if(_error != 0)
    {
        os << "\nsyscall exception: " << IceUtilInternal::errorToString(_error);
    }
}

string
IceUtil::FileLockException::ice_id() const
{
    return "::IceUtil::FileLockException";
}

int
IceUtil::FileLockException::error() const
{
    return _error;
}

IceUtil::OptionalNotSetException::OptionalNotSetException(const char* file, int line) :
    ExceptionHelper<OptionalNotSetException>(file, line)
{
    if(IceUtilInternal::nullHandleAbort)
    {
        abort();
    }
}

string
IceUtil::OptionalNotSetException::ice_id() const
{
    return "::IceUtil::OptionalNotSetException";
}
