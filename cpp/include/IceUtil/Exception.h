// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_EXCEPTION_H
#define ICE_UTIL_EXCEPTION_H

#include <IceUtil/Config.h>

namespace IceUtil
{

class Exception
{
public:

    Exception();
    Exception(const char*, int);
    virtual ~Exception();
    virtual std::string _name() const;
    virtual void _print(std::ostream&) const;
    virtual Exception* _clone() const;
    virtual void _throw() const;
    const char* _file() const;
    int _line() const;
    
private:
    
    const char* _theFile;
    int _theLine;
};

std::ostream& operator<<(std::ostream&, const Exception&);

class NullHandleException : public Exception
{
public:
    
    NullHandleException(const char*, int);
    virtual std::string _name() const;
    virtual std::string _description() const;
    virtual Exception* _clone() const;
    virtual void _throw() const;
};

}

#endif
