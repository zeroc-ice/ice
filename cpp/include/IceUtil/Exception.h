// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_EXCEPTION_H
#define ICE_UTIL_EXCEPTION_H

#include <IceUtil/Config.h>

#include <exception>
#include <vector>

namespace IceUtil
{

class ICE_API Exception : public std::exception
{
public:

    Exception();
    Exception(const char*, int);
#ifndef ICE_CPP11_COMPILER
    virtual ~Exception() throw() = 0;
#endif
    virtual std::string ice_id() const = 0;
    virtual void ice_print(std::ostream&) const;
    virtual const char* what() const ICE_NOEXCEPT;
#ifdef ICE_CPP11_MAPPING
    std::unique_ptr<Exception> ice_clone() const;
#else
    virtual Exception* ice_clone() const = 0;
    ICE_DEPRECATED_API("ice_name() is deprecated, use ice_id() instead.")
    std::string ice_name() const;
#endif
    virtual void ice_throw() const = 0;

    const char* ice_file() const;
    int ice_line() const;
    std::string ice_stackTrace() const;

protected:

#ifdef ICE_CPP11_MAPPING
    virtual Exception* ice_cloneImpl() const = 0;
#endif

private:

    const char* _file;
    int _line;
    const std::vector<void*> _stackFrames;
    mutable ::std::string _str; // Initialized lazily in what().
};

ICE_API std::ostream& operator<<(std::ostream&, const Exception&);

#ifdef ICE_CPP11_MAPPING

template<typename E, typename B = Exception>
class ExceptionHelper : public B
{
public:

    using B::B;

    std::unique_ptr<E> ice_clone() const
    {
        return std::unique_ptr<E>(static_cast<E*>(ice_cloneImpl()));
    }

    virtual void ice_throw() const override
    {
        throw static_cast<const E&>(*this);
    }

protected:

    virtual Exception* ice_cloneImpl() const override
    {
        return new E(static_cast<const E&>(*this));
    }
};

#else // C++98 mapping

template<typename E>
class ExceptionHelper : public Exception
{
public:

    ExceptionHelper()
    {
    }

    ExceptionHelper(const char* file, int line) : Exception(file, line)
    {
    }

    virtual void ice_throw() const
    {
        throw static_cast<const E&>(*this);
    }
};

#endif

class ICE_API NullHandleException : public ExceptionHelper<NullHandleException>
{
public:

    NullHandleException(const char*, int);
    virtual std::string ice_id() const;

#ifndef ICE_CPP11_MAPPING
    virtual NullHandleException* ice_clone() const;
#endif
};

class ICE_API IllegalArgumentException : public ExceptionHelper<IllegalArgumentException>
{
public:

    IllegalArgumentException(const char*, int);
    IllegalArgumentException(const char*, int, const std::string&);

#ifndef ICE_CPP11_COMPILER
    virtual ~IllegalArgumentException() throw();
#endif

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;

#ifndef ICE_CPP11_MAPPING
    virtual IllegalArgumentException* ice_clone() const;
#endif

    std::string reason() const;

private:

    const std::string _reason;
};

//
// IllegalConversionException is raised to report a string conversion error
//
class ICE_API IllegalConversionException : public ExceptionHelper<IllegalConversionException>
{
public:

    IllegalConversionException(const char*, int);
    IllegalConversionException(const char*, int, const std::string&);

#ifndef ICE_CPP11_COMPILER
    virtual ~IllegalConversionException() throw();
#endif

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;

#ifndef ICE_CPP11_MAPPING
    virtual IllegalConversionException* ice_clone() const;
#endif

    std::string reason() const;

private:

    const std::string _reason;
};

class ICE_API SyscallException : public ExceptionHelper<SyscallException>
{
public:

    SyscallException(const char*, int, int);

#ifndef ICE_CPP11_COMPILER
    virtual ~SyscallException() throw();
#endif

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;

#ifndef ICE_CPP11_MAPPING
    virtual SyscallException* ice_clone() const;
#endif

    int error() const;

private:

    const int _error;
};

#ifdef ICE_CPP11_MAPPING

template<typename E>
using SyscallExceptionHelper = ExceptionHelper<E, SyscallException>;

#else // C++98 mapping

template<typename E>
class SyscallExceptionHelper : public SyscallException
{
public:

    SyscallExceptionHelper(const char* file, int line, int errorCode) :
        SyscallException(file, line, errorCode)
    {
    }

    virtual void ice_throw() const
    {
        throw static_cast<const E&>(*this);
    }
};

#endif

class ICE_API FileLockException : public ExceptionHelper<FileLockException>
{
public:

    FileLockException(const char*, int, int, const std::string&);

#ifndef ICE_CPP11_COMPILER
    virtual ~FileLockException() throw();
#endif

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;

#ifndef ICE_CPP11_MAPPING
    virtual FileLockException* ice_clone() const;
#endif

    std::string path() const;
    int error() const;

private:

    const int _error;
    std::string _path;
};

class ICE_API OptionalNotSetException : public ExceptionHelper<OptionalNotSetException>
{
public:

    OptionalNotSetException(const char*, int);
    virtual std::string ice_id() const;

#ifndef ICE_CPP11_MAPPING
    virtual OptionalNotSetException* ice_clone() const;
#endif
};

}

namespace IceUtilInternal
{

enum StackTraceImpl { STNone, STDbghelp, STLibbacktrace, STLibbacktracePlus, STBacktrace };

ICE_API StackTraceImpl stackTraceImpl();

}

#endif
