// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#if defined(_MSC_VER) && _MSC_VER >= 1700
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

#include <IceUtil/Exception.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/StringUtil.h>
#include <ostream>
#include <iomanip>
#include <cstdlib>


#if defined(__GNUC__) && !defined(__sun) && !defined(__FreeBSD__) && !defined(__MINGW32__) && \
    !defined(ICE_STATIC_LIBS)
#  include <execinfo.h>
#  include <cxxabi.h>
#  define ICE_GCC_STACK_TRACES
#endif

#if defined(_WIN32) && !defined(ICE_OS_WINRT) && !defined(__MINGW32__)
#  define ICE_WIN32_STACK_TRACES
#  if defined(_MSC_VER) && _MSC_VER >= 1700
#    define DBGHELP_TRANSLATE_TCHAR
#    include <IceUtil/StringConverter.h>
#    if _MSC_VER >= 1900
#       // VS 2015 RC issues this warning for code in DbgHelp.h
#       pragma warning(disable:4091)
#    endif
#  endif
#  include <DbgHelp.h>
#  include <tchar.h>
#endif

using namespace std;

namespace IceUtilInternal
{

bool ICE_UTIL_API printStackTraces = false;
bool ICE_UTIL_API nullHandleAbort = false;

}

namespace
{

IceUtil::Mutex* globalMutex = 0;

#ifdef ICE_WIN32_STACK_TRACES
HANDLE process = 0;
#endif

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
#ifdef ICE_WIN32_STACK_TRACES
        if(process != 0)
        {
            SymCleanup(process);
            process = 0;
        }
#endif
    }
};

Init init;


vector<void*>
getStackFrames()
{
    vector<void*> stackFrames;

    if(!IceUtilInternal::printStackTraces)
    {
        return stackFrames;
    }

#  if defined(ICE_WIN32_STACK_TRACES)

    stackFrames.resize(61);
    //
    // 1: skip the first frame (the call to getStackFrames)
    // 1 + stackSize < 63 on Windows XP according to the documentation for CaptureStackBackTrace
    //
    USHORT frameCount = CaptureStackBackTrace(1, static_cast<DWORD>(stackFrames.size()), &stackFrames.front(), 0);

    stackFrames.resize(frameCount);

#  elif defined(ICE_GCC_STACK_TRACES)
    
    stackFrames.resize(100);
    size_t stackDepth = backtrace(&stackFrames.front(), stackFrames.size());
    stackFrames.resize(stackDepth);

#  endif

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

#  if defined(ICE_WIN32_STACK_TRACES)

    //
    // Note: the Sym functions are not thread-safe
    //
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);
    if(process == 0)
    {
        //
        // Compute Search path (best effort)
        // consists of the current working directory, this DLL (or exe) directory and %_NT_SYMBOL_PATH%
        //
        basic_string<TCHAR> searchPath;
        const TCHAR pathSeparator = _T('\\');
        const TCHAR searchPathSeparator = _T(';');

        TCHAR cwd[MAX_PATH];
        if(GetCurrentDirectory(MAX_PATH, cwd) != 0)
        {
            searchPath = cwd;
        }

        HMODULE myModule = 0;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           "startHook",
                           &myModule);
        //
        // If GetModuleHandleEx fails, myModule is NULL, i.e. we'll locate the current exe's directory.
        //

        TCHAR myFilename[MAX_PATH];
        DWORD len = GetModuleFileName(myModule, myFilename, MAX_PATH);
        if(len != 0 && len < MAX_PATH)
        {
            assert(myFilename[len] == 0);

            basic_string<TCHAR> myPath = myFilename;
            size_t pos = myPath.find_last_of(pathSeparator);
            if(pos != basic_string<TCHAR>::npos)
            {
                myPath = myPath.substr(0, pos);

                if(!searchPath.empty())
                {
                    searchPath += searchPathSeparator;
                }
                searchPath += myPath;
            }
        }

        const DWORD size = 1024;
        TCHAR symbolPath[size];
        len = GetEnvironmentVariable(_T("_NT_SYMBOL_PATH"), symbolPath, size);
        if(len > 0 && len < size)
        {
            if(!searchPath.empty())
            {
                searchPath += searchPathSeparator;
            }
            searchPath += symbolPath;
        }

        process = GetCurrentProcess();

        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS | SYMOPT_EXACT_SYMBOLS | SYMOPT_UNDNAME);
        if(SymInitialize(process, searchPath.c_str(), TRUE) == 0)
        {
            process = 0;
            return "No stack trace: SymInitialize failed with " + IceUtilInternal::errorToString(GetLastError());
        }
    }
    lock.release();

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#   if defined(DBGHELP_TRANSLATE_TCHAR)
    static_assert(sizeof(TCHAR) == sizeof(wchar_t), "Bad TCHAR - should be wchar_t");
#   else
    static_assert(sizeof(TCHAR) == sizeof(char), "Bad TCHAR - should be char");
#  endif
#endif

    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    
    SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_SYM_NAME;
    
    IMAGEHLP_LINE64 line = {};
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    DWORD displacement = 0;
    
    lock.acquire();
    
    // TODO: call SymRefreshModuleList here? (not available on XP)
    
#ifdef DBGHELP_TRANSLATE_TCHAR
    const IceUtil::StringConverterPtr converter = IceUtil::getProcessStringConverter();
#endif
    for(size_t i = 0; i < stackFrames.size(); i++)
    {
        if(!stackTrace.empty())
        {
            stackTrace += "\n";
        }
        
        stringstream s;
        s << setw(3) << i << " ";
        
        DWORD64 address = reinterpret_cast<DWORD64>(stackFrames[i]);
        
        BOOL ok = SymFromAddr(process, address, 0, symbol);
        if(ok)
        {
#ifdef DBGHELP_TRANSLATE_TCHAR
            s << IceUtil::wstringToString(symbol->Name, converter);
#else
            s << symbol->Name;
#endif
            ok = SymGetLineFromAddr64(process, address, &displacement, &line);
            if(ok)
            {
                s << " at line " << line.LineNumber << " in "
#ifdef DBGHELP_TRANSLATE_TCHAR
                  << IceUtil::wstringToString(line.FileName, converter);
#else
                  << line.FileName;
#endif
            }
        }
        else
        {
            s << hex << "0x" << address;
        }
        stackTrace += s.str();
    }
    lock.release();

#  elif defined(ICE_GCC_STACK_TRACES)

    // With some compilers/toolchains this can fail so we must check that
    // stackStrings is not null.
    
    char** stackStrings = backtrace_symbols(&stackFrames.front(), stackFrames.size());
    if(stackStrings != 0)
    {
        //
        // Start at 1 to skip the top frame (== call to this function)
        //
        for(size_t i = 1; i < stackFrames.size(); i++)
        {
            string line(stackStrings[i]);

            if(i > 1)
            {
                stackTrace += "\n";
            }

            stringstream s;
            s << setw(3) << i - 1 << " ";

            //
            // For each line attempt to parse the mangled function name as well
            // as the source library/executable.
            //
            string mangled;
            string source;
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
                        mangled = tmp.substr(0 , plus);

                        source = line.substr(0, openParen);
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
                                mangled = tmp;

                                source = line.substr(start, finish - start);
                            }
                        }
                    }
                }
            }
            if(mangled.size() != 0)
            {
                //
                // Unmangle the function name
                //
                int status;
                char* unmangled = abi::__cxa_demangle(mangled.c_str(), 0, 0, &status);
                if(unmangled)
                {
                    s << unmangled;
                    free(unmangled);
                }
                else
                {
                    s << mangled << "()";
                }

                if(!source.empty())
                {
                    s << " in " << source;
                }
            }
            else
            {
                s << line;
            }

            stackTrace += s.str();
        }
        free(stackStrings);
    }
    else
    {
        stackTrace = "<stack trace unavailable>";
    }

#  endif

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

IceUtil::Exception::~Exception() throw()
{
}

const char* IceUtil::Exception::_name = "IceUtil::Exception";

string
IceUtil::Exception::ice_name() const
{
    return _name;
}

void
IceUtil::Exception::ice_print(ostream& out) const
{
    if(_file && _line > 0)
    {
        out << _file << ':' << _line << ": ";
    }
    out << ice_name();
}

const char*
IceUtil::Exception::what() const throw()
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

IceUtil::Exception*
IceUtil::Exception::ice_clone() const
{
    return new Exception(*this);
}

void
IceUtil::Exception::ice_throw() const
{
    throw *this;
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

ostream&
IceUtil::operator<<(ostream& out, const IceUtil::Exception& ex)
{
    ex.ice_print(out);
    return out;
}

IceUtil::NullHandleException::NullHandleException(const char* file, int line) :
    Exception(file, line)
{
    if(IceUtilInternal::nullHandleAbort)
    {
        abort();
    }
}

IceUtil::NullHandleException::~NullHandleException() throw()
{
}

const char* IceUtil::NullHandleException::_name = "IceUtil::NullHandleException";

string
IceUtil::NullHandleException::ice_name() const
{
    return _name;
}

IceUtil::NullHandleException*
IceUtil::NullHandleException::ice_clone() const
{
    return new NullHandleException(*this);
}

void
IceUtil::NullHandleException::ice_throw() const
{
    throw *this;
}

IceUtil::IllegalArgumentException::IllegalArgumentException(const char* file, int line) :
    Exception(file, line)
{
}

IceUtil::IllegalArgumentException::IllegalArgumentException(const char* file, int line, const string& r) :
    Exception(file, line),
    _reason(r)
{
}

IceUtil::IllegalArgumentException::~IllegalArgumentException() throw()
{
}

const char* IceUtil::IllegalArgumentException::_name = "IceUtil::IllegalArgumentException";

string
IceUtil::IllegalArgumentException::ice_name() const
{
    return _name;
}

void
IceUtil::IllegalArgumentException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ": " << _reason;
}

IceUtil::IllegalArgumentException*
IceUtil::IllegalArgumentException::ice_clone() const
{
    return new IllegalArgumentException(*this);
}

void
IceUtil::IllegalArgumentException::ice_throw() const
{
    throw *this;
}

string
IceUtil::IllegalArgumentException::reason() const
{
    return _reason;
}

//
// IllegalConversionException
//

const char* IceUtil::IllegalConversionException::_name = "IceUtil::IllegalConversionException";

IceUtil::IllegalConversionException::IllegalConversionException(const char* file, int line):
    Exception(file, line)
{}

IceUtil::IllegalConversionException::IllegalConversionException(const char* file, int line,
                                                                const string& reason):
    Exception(file, line),
    _reason(reason)
{}

IceUtil::IllegalConversionException::~IllegalConversionException() throw()
{}

string
IceUtil::IllegalConversionException::ice_name() const
{
    return _name;
}

void
IceUtil::IllegalConversionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ": " << _reason;

}

IceUtil::IllegalConversionException*
IceUtil::IllegalConversionException::ice_clone() const
{
    return new IllegalConversionException(*this);
}

void
IceUtil::IllegalConversionException::ice_throw() const
{
    throw *this;
}

string
IceUtil::IllegalConversionException::reason() const
{
    return _reason;
}



IceUtil::SyscallException::SyscallException(const char* file, int line, int err ):
    Exception(file, line),
    _error(err)
{
}

const char* IceUtil::SyscallException::_name = "IceUtil::SyscallException";

string
IceUtil::SyscallException::ice_name() const
{
    return _name;
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

IceUtil::SyscallException*
IceUtil::SyscallException::ice_clone() const
{
    return new SyscallException(*this);
}

void
IceUtil::SyscallException::ice_throw() const
{
    throw *this;
}

int
IceUtil::SyscallException::error() const
{
    return _error;
}


IceUtil::FileLockException::FileLockException(const char* file, int line, int err, const string& path):
    Exception(file, line),
    _error(err),
    _path(path)
{
}

IceUtil::FileLockException::~FileLockException() throw()
{
}

const char* IceUtil::FileLockException::_name = "IceUtil::FileLockedException";

string
IceUtil::FileLockException::ice_name() const
{
    return _name;
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

IceUtil::FileLockException*
IceUtil::FileLockException::ice_clone() const
{
    return new FileLockException(*this);
}

void
IceUtil::FileLockException::ice_throw() const
{
    throw *this;
}

int
IceUtil::FileLockException::error() const
{
    return _error;
}

IceUtil::OptionalNotSetException::OptionalNotSetException(const char* file, int line) :
    Exception(file, line)
{
    if(IceUtilInternal::nullHandleAbort)
    {
        abort();
    }
}

IceUtil::OptionalNotSetException::~OptionalNotSetException() throw()
{
}

const char* IceUtil::OptionalNotSetException::_name = "IceUtil::OptionalNotSetException";

string
IceUtil::OptionalNotSetException::ice_name() const
{
    return _name;
}

IceUtil::OptionalNotSetException*
IceUtil::OptionalNotSetException::ice_clone() const
{
    return new OptionalNotSetException(*this);
}

void
IceUtil::OptionalNotSetException::ice_throw() const
{
    throw *this;
}

#ifndef _WIN32
IceUtil::IconvInitializationException::IconvInitializationException(const char* file, int line, const string& reason) :
    Exception(file, line),
    _reason(reason)
{
}

IceUtil::IconvInitializationException::~IconvInitializationException() throw()
{
}

const char* IceUtil::IconvInitializationException::_name = "IceUtil::IconvInitializationException";

string
IceUtil::IconvInitializationException::ice_name() const
{
    return _name;
}

void
IceUtil::IconvInitializationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ": " << _reason;
}

IceUtil::IconvInitializationException*
IceUtil::IconvInitializationException::ice_clone() const
{
    return new IconvInitializationException(*this);
}

void
IceUtil::IconvInitializationException::ice_throw() const
{
    throw *this;
}

string
IceUtil::IconvInitializationException::reason() const
{
    return _reason;
}
#endif
