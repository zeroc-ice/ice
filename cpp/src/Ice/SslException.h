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
class ICE_API SecurityException : public LocalException
{
public:    

    SecurityException(const char*, const char*, int);
    SecurityException(const SecurityException&);
    SecurityException& operator=(const SecurityException&);
    virtual string toString() const;
    virtual string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    string _message;
};

namespace Ssl
{

class ICE_API ConfigParseException : public SecurityException
{
public:    

    ConfigParseException(const char*, const char*, int);
    ConfigParseException(const ConfigParseException&);
    ConfigParseException& operator=(const ConfigParseException&);
    virtual string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

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
    virtual string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

};

class ICE_API ProtocolException : public ShutdownException
{
public:    

    ProtocolException(const char*, const char*, int);
    ProtocolException(const ProtocolException&);
    ProtocolException& operator=(const ProtocolException&);
    virtual string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

};

class ICE_API CertificateException : public ShutdownException
{
public:    

    CertificateException(const char*, const char*, int);
    CertificateException(const CertificateException&);
    CertificateException& operator=(const CertificateException&);
    virtual string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

};

namespace OpenSSL
{

// ContextException
//
// This exception indicates that a problem occurred while setting up the
// SSL context structure (SSL_CTX).
//
class ICE_API ContextException : public SecurityException
{
public:    

    ContextException(const char*, const char*, int);
    ContextException(const ContextException&);
    ContextException& operator=(const ContextException&);
    virtual string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

};

}

}

}

#endif
