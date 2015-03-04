// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_EXCEPTION_H
#define ICE_UTIL_EXCEPTION_H

#include <IceUtil/Config.h>

#include <exception>

namespace IceUtil
{

class ICE_UTIL_API Exception : public std::exception
{
public:

    Exception();
    Exception(const char*, int);

    virtual ~Exception() throw();
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual const char* what() const throw();
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

    const char* ice_file() const;
    int ice_line() const;
    const std::string& ice_stackTrace() const;
    
private:
    
    const char* _file;
    int _line;
    static const char* _name;
    const std::string _stackTrace;
    mutable ::std::string _str; // Initialized lazily in what().
};

ICE_UTIL_API std::ostream& operator<<(std::ostream&, const Exception&);

class ICE_UTIL_API NullHandleException : public Exception
{
public:
    
    NullHandleException(const char*, int);
    virtual ~NullHandleException() throw();
    virtual std::string ice_name() const;
    virtual NullHandleException* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

class ICE_UTIL_API IllegalArgumentException : public Exception
{
public:
    
    IllegalArgumentException(const char*, int);
    IllegalArgumentException(const char*, int, const std::string&);
    virtual ~IllegalArgumentException() throw();
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual IllegalArgumentException* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason() const;

private:

    static const char* _name;
    std::string _reason;
};

class ICE_UTIL_API SyscallException : public Exception
{
public:

    SyscallException(const char*, int, int);
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual SyscallException* ice_clone() const;
    virtual void ice_throw() const;

    int error() const;

private:

    const int _error;
    static const char* _name;
};

class ICE_UTIL_API FileLockException : public Exception
{
public:

    FileLockException(const char*, int, int, const std::string&);
    virtual ~FileLockException() throw();
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual FileLockException* ice_clone() const;
    virtual void ice_throw() const;

    std::string path() const;
    int error() const;

private:

    const int _error;
    static const char* _name;
    std::string _path;
};

class ICE_UTIL_API OptionalNotSetException : public Exception
{
public:
    
    OptionalNotSetException(const char*, int);
    virtual ~OptionalNotSetException() throw();
    virtual std::string ice_name() const;
    virtual OptionalNotSetException* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

}

#endif
