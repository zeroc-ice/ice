////*********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
////*********************************************************************

#ifndef ICE_SSL_EXCEPTION_H
#define ICE_SSL_EXCEPTION_H

#include <Ice/LocalException.h>
#include <string>
#include <ostream>

namespace IceSecurity
{

using std::string;
using std::ostream;
using Ice::Exception;
using Ice::LocalException;

// SecurityException
//
// This exception serves as the base for all SSL related exceptions.  It should probably
// not be constructed directly, but can be used in this manner if desired.
//
class ICE_API SecurityException : public LocalException // public SocketException
{
public:    

    SecurityException(const char*, const char*, int);
    SecurityException(const SecurityException&);
    SecurityException& operator=(const SecurityException&);
    virtual string toString() const;
    virtual string _ice_name() const;
    virtual void _ice_print(std::ostream&) const;
    virtual Exception* _ice_clone() const;
    virtual void _ice_throw() const;

private:

    string _message;
};

namespace Ssl
{

// InitException
//
// This exception indicates a problem during SSL connection setup, most likely due to
// problems with the SSL handshake.  This is primarily a client-side exception.
//
class ICE_API InitException : public SecurityException
{
public:    

    InitException(const char*, const char*, int);
    InitException(const InitException&);
    InitException& operator=(const InitException&);
    virtual string _ice_name() const;
    virtual void _ice_print(std::ostream&) const;
    virtual Exception* _ice_clone() const;
    virtual void _ice_throw() const;

};

// ReInitException
//
// This exception indicates that the SSL connection should be re-initialized.
// This is primarily a server-side exception.
//
class ICE_API ReInitException : public SecurityException
{
public:    

    ReInitException(const char*, const char*, int);
    ReInitException(const ReInitException&);
    ReInitException& operator=(const ReInitException&);
    virtual string _ice_name() const;
    virtual void _ice_print(std::ostream&) const;
    virtual Exception* _ice_clone() const;
    virtual void _ice_throw() const;

};

// ShutdownException
//
// This exception indicates that there has been a fatal SSL connection problem
// that requires the shutdown of the connection and closing of the underlying
// transport.
//
class ICE_API ShutdownException : public SecurityException
{
public:    

    ShutdownException(const char*, const char*, int);
    ShutdownException(const ShutdownException&);
    ShutdownException& operator=(const ShutdownException&);
    virtual string _ice_name() const;
    virtual void _ice_print(std::ostream&) const;
    virtual Exception* _ice_clone() const;
    virtual void _ice_throw() const;

};

namespace OpenSSL
{

// ContextException
//
// This exception indicates that a problem occurred while setting up the
// SSL context structure (SSL_CTX).
//
class ICE_API ContextException : public InitException
{
public:    

    ContextException(const char*, const char*, int);
    ContextException(const ContextException&);
    ContextException& operator=(const ContextException&);
    virtual string _ice_name() const;
    virtual void _ice_print(std::ostream&) const;
    virtual Exception* _ice_clone() const;
    virtual void _ice_throw() const;

};

}

}

}

#endif
