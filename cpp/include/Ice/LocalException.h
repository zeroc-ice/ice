// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOCAL_EXCEPTION_H
#define ICE_LOCAL_EXCEPTION_H

#include <Ice/Config.h>

namespace Ice
{

class ICE_API LocalException
{
public:

    virtual ~LocalException();
    virtual std::string toString() const;
    virtual LocalException* clone() const;
};

ICE_API std::ostream& operator<<(std::ostream&, const LocalException&);

class ICE_API SystemException : public LocalException
{
public:    

    SystemException();
    SystemException(const SystemException&);
    SystemException& operator=(const SystemException&);

    virtual std::string toString() const;
    virtual LocalException* clone() const;

protected:

    int error;
};

class ICE_API SocketException : public SystemException
{
public:    

    virtual std::string toString() const;
    virtual LocalException* clone() const;
};

class ICE_API ConnectFailedException : public SocketException
{
public:    

    virtual std::string toString() const;
    virtual LocalException* clone() const;
};

class ICE_API ConnectionLostException : public SocketException
{
public:    

    virtual std::string toString() const;
    virtual LocalException* clone() const;
};

class ICE_API DNSException : public SystemException
{
public:    

    DNSException();
    DNSException(const DNSException&);
    DNSException& operator=(const DNSException&);

    virtual std::string toString() const;
    virtual LocalException* clone() const;
};

class ICE_API ProtocolException : public LocalException
{
};

class ICE_API UnmarshalException : public ProtocolException
{
};

class ICE_API OutOfBoundsUnmarshalException : public UnmarshalException
{
public:    

    virtual std::string toString() const;
    virtual LocalException* clone() const;
};

}

#endif
