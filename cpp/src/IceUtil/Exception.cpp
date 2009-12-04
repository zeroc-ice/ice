// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Exception.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/StringUtil.h>
#include <ostream>
#include <cstdlib>

#if defined(__GNUC__) && !defined(__sun)
#  include <execinfo.h>
#  include <cxxabi.h>
#endif

using namespace std;

namespace IceUtilInternal
{

bool ICE_DECLSPEC_EXPORT printStackTraces = false;
bool ICE_DECLSPEC_EXPORT nullHandleAbort = false;

}

namespace
{

IceUtil::Mutex* globalMutex = 0;

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
    }
};

Init init;

#if defined(__GNUC__) && !defined(__sun)
string
getStackTrace()
{
    string stackTrace;

    if(!IceUtilInternal::printStackTraces)
    {
        return stackTrace;
    }

    const size_t maxDepth = 100;
    void *stackAddrs[maxDepth];

    size_t stackDepth = backtrace(stackAddrs, maxDepth);
    char **stackStrings = backtrace_symbols(stackAddrs, stackDepth);

    bool checkException = true;
    for (size_t i = 1; i < stackDepth; i++)
    {
        string line(stackStrings[i]);

        //
        // Don't add the traces for the Exception constructors.
        //
        if(checkException)
        {
            if(line.find("ExceptionC") != string::npos)
            {
                continue;
            }
            else
            {
                checkException = false;
            }
        }
        else
        {
            stackTrace += "\n";
        }

        stackTrace += "  ";
        
        //
        // For each line attempt to parse the mangled function name as well
        // as the source library/executable.
        //
        string mangled;
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

                    stackTrace += line.substr(0, openParen); 
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

                            stackTrace += line.substr(start, finish - start);
                        }
                    }
                }
	    }
        }
        if(mangled.size() != 0)
        {
            stackTrace += ": ";

            //
            // Unmangle the function name
            //
            int status;
            char* unmangled = abi::__cxa_demangle(mangled.c_str(), 0, 0, &status);
            if(unmangled)
            {
                stackTrace += unmangled;
                free(unmangled);
            }
            else
            {
                stackTrace += mangled;
                stackTrace += "()";
            }
        }
        else
        {
            stackTrace += line;
        }
    }
    free(stackStrings);

    return stackTrace;
}
#endif

}

IceUtil::Exception::Exception() :
    _file(0),
    _line(0)
#if defined(__GNUC__) && !defined(__sun)
    , _stackTrace(getStackTrace())
#endif
{
}
    
IceUtil::Exception::Exception(const char* file, int line) :
    _file(file),
    _line(line)
#if defined(__GNUC__) && !defined(__sun)
    , _stackTrace(getStackTrace())
#endif
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

const string&
IceUtil::Exception::ice_stackTrace() const
{
    return _stackTrace;
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

IceUtil::Exception*
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

IceUtil::Exception*
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

IceUtil::Exception*
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

IceUtil::Exception*
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
