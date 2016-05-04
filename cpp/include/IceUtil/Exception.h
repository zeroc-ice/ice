// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

class ICE_UTIL_API Exception : public std::exception
{
public:

    Exception();
    Exception(const char*, int);

    virtual ~Exception() ICE_NOEXCEPT;
    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
    virtual const char* what() const ICE_NOEXCEPT;
#ifdef ICE_CPP11_MAPPING
    std::exception_ptr ice_clone() const;
#else
    ICE_DEPRECATED_API("ice_name() is deprecated, use ice_id() instead.")
    std::string ice_name() const;
    virtual Exception* ice_clone() const;
#endif
    virtual void ice_throw() const;

    const char* ice_file() const;
    int ice_line() const;
    std::string ice_stackTrace() const;

private:

    const char* _file;
    int _line;
    const std::vector<void*> _stackFrames;
    mutable ::std::string _str; // Initialized lazily in what().
};

ICE_UTIL_API std::ostream& operator<<(std::ostream&, const Exception&);

class ICE_UTIL_API NullHandleException : public Exception
{
public:

    NullHandleException(const char*, int);
    virtual ~NullHandleException() ICE_NOEXCEPT;
    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual NullHandleException* ice_clone() const;
#endif
    virtual void ice_throw() const;
};

class ICE_UTIL_API IllegalArgumentException : public Exception
{
public:

    IllegalArgumentException(const char*, int);
    IllegalArgumentException(const char*, int, const std::string&);
    virtual ~IllegalArgumentException() ICE_NOEXCEPT;
    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual IllegalArgumentException* ice_clone() const;
#endif
    virtual void ice_throw() const;

    std::string reason() const;

private:

    const std::string _reason;
};

//
// IllegalConversionException is raised to report a string conversion error
//
class ICE_UTIL_API IllegalConversionException : public Exception
{
public:

    IllegalConversionException(const char*, int);
    IllegalConversionException(const char*, int, const std::string&);
    virtual ~IllegalConversionException() ICE_NOEXCEPT;
    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual IllegalConversionException* ice_clone() const;
#endif
    virtual void ice_throw() const;

    std::string reason() const;
private:

    const std::string _reason;
};


class ICE_UTIL_API SyscallException : public Exception
{
public:

    SyscallException(const char*, int, int);
    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual SyscallException* ice_clone() const;
#endif
    virtual void ice_throw() const;

    int error() const;

private:

    const int _error;
};

class ICE_UTIL_API FileLockException : public Exception
{
public:

    FileLockException(const char*, int, int, const std::string&);
    virtual ~FileLockException() ICE_NOEXCEPT;
    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual FileLockException* ice_clone() const;
#endif
    virtual void ice_throw() const;

    std::string path() const;
    int error() const;

private:

    const int _error;
    std::string _path;
};

class ICE_UTIL_API OptionalNotSetException : public Exception
{
public:

    OptionalNotSetException(const char*, int);
    virtual ~OptionalNotSetException() ICE_NOEXCEPT;
    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual OptionalNotSetException* ice_clone() const;
#endif
    virtual void ice_throw() const;
};

#ifndef _WIN32
class ICE_UTIL_API IconvInitializationException : public Exception
{
public:

    IconvInitializationException(const char*, int, const std::string&);
    virtual ~IconvInitializationException() ICE_NOEXCEPT;
    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual IconvInitializationException* ice_clone() const;
#endif
    virtual void ice_throw() const;

    std::string reason() const;

private:

    std::string _reason;
};
#endif

}

namespace IceUtilInternal
{
  ICE_UTIL_API bool canCaptureStackTrace();
}

#endif
