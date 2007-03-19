// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_EXCEPTION_H
#define ICE_UTIL_EXCEPTION_H

#include <IceUtil/Config.h>

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
    
private:
    
    const char* _file;
    int _line;
    static const char* _name;
    mutable ::std::string _str; // Initialized lazily in what().
};

ICE_UTIL_API std::ostream& operator<<(std::ostream&, const Exception&);

class ICE_UTIL_API NullHandleException : public Exception
{
public:
    
    NullHandleException(const char*, int);
    virtual ~NullHandleException() throw();
    virtual std::string ice_name() const;
    virtual Exception* ice_clone() const;
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
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason;

private:

    static const char* _name;
};

ICE_UTIL_API std::ostream& operator<<(std::ostream&, const IllegalArgumentException&);

}

#endif
