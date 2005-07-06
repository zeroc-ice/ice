// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOCAL_EXCEPTION_H
#define ICEE_LOCAL_EXCEPTION_H

#include <IceE/Exception.h>
#include <IceE/Identity.h>
#include <IceE/BuiltinSequences.h>

namespace Ice
{

class ICEE_API AlreadyRegisteredException : public ::Ice::LocalException
{
public:

    AlreadyRegisteredException(const char*, int);
    AlreadyRegisteredException(const char*, int, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string kindOfObject;
    ::std::string id;
};

class ICEE_API NotRegisteredException : public ::Ice::LocalException
{
public:

    NotRegisteredException(const char*, int);
    NotRegisteredException(const char*, int, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string kindOfObject;
    ::std::string id;
};

class ICEE_API TwowayOnlyException : public ::Ice::LocalException
{
public:

    TwowayOnlyException(const char*, int);
    TwowayOnlyException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string operation;
};

class ICEE_API CloneNotImplementedException : public ::Ice::LocalException
{
public:

    CloneNotImplementedException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API UnknownException : public ::Ice::LocalException
{
public:

    UnknownException(const char*, int);
    UnknownException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string unknown;
};

class ICEE_API UnknownLocalException : public ::Ice::UnknownException
{
public:

    UnknownLocalException(const char*, int);
    UnknownLocalException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API UnknownUserException : public ::Ice::UnknownException
{
public:

    UnknownUserException(const char*, int);
    UnknownUserException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API VersionMismatchException : public ::Ice::LocalException
{
public:

    VersionMismatchException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API CommunicatorDestroyedException : public ::Ice::LocalException
{
public:

    CommunicatorDestroyedException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

#ifndef ICEE_PURE_CLIENT

class ICEE_API ObjectAdapterDeactivatedException : public ::Ice::LocalException
{
public:

    ObjectAdapterDeactivatedException(const char*, int);
    ObjectAdapterDeactivatedException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string name;
};

class ICEE_API ObjectAdapterIdInUseException : public ::Ice::LocalException
{
public:

    ObjectAdapterIdInUseException(const char*, int);
    ObjectAdapterIdInUseException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string id;
};
#endif

class ICEE_API NoEndpointException : public ::Ice::LocalException
{
public:

    NoEndpointException(const char*, int);
    NoEndpointException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string proxy;
};

class ICEE_API EndpointParseException : public ::Ice::LocalException
{
public:

    EndpointParseException(const char*, int);
    EndpointParseException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string str;
};

class ICEE_API IdentityParseException : public ::Ice::LocalException
{
public:

    IdentityParseException(const char*, int);
    IdentityParseException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string str;
};

class ICEE_API ProxyParseException : public ::Ice::LocalException
{
public:

    ProxyParseException(const char*, int);
    ProxyParseException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string str;
};

class ICEE_API IllegalIdentityException : public ::Ice::LocalException
{
public:

    IllegalIdentityException(const char*, int);
    IllegalIdentityException(const char*, int, const ::Ice::Identity&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::Ice::Identity id;
};

class ICEE_API RequestFailedException : public ::Ice::LocalException
{
public:

    RequestFailedException(const char*, int);
    RequestFailedException(const char*, int, const ::Ice::Identity&, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::Ice::Identity id;
    ::std::string facet;
    ::std::string operation;
};

class ICEE_API ObjectNotExistException : public ::Ice::RequestFailedException
{
public:

    ObjectNotExistException(const char*, int);
    ObjectNotExistException(const char*, int, const ::Ice::Identity&, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API FacetNotExistException : public ::Ice::RequestFailedException
{
public:

    FacetNotExistException(const char*, int);
    FacetNotExistException(const char*, int, const ::Ice::Identity&, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API OperationNotExistException : public ::Ice::RequestFailedException
{
public:

    OperationNotExistException(const char*, int);
    OperationNotExistException(const char*, int, const ::Ice::Identity&, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API SyscallException : public ::Ice::LocalException
{
public:

    SyscallException(const char*, int);
    SyscallException(const char*, int, ::Ice::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::Ice::Int error;
};

class ICEE_API SocketException : public ::Ice::SyscallException
{
public:

    SocketException(const char*, int);
    SocketException(const char*, int, ::Ice::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API FileException : public ::Ice::SyscallException
{
public:

    FileException(const char*, int);
    FileException(const char*, int, ::Ice::Int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string path;
};

class ICEE_API ConnectFailedException : public ::Ice::SocketException
{
public:

    ConnectFailedException(const char*, int);
    ConnectFailedException(const char*, int, ::Ice::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ConnectionRefusedException : public ::Ice::ConnectFailedException
{
public:

    ConnectionRefusedException(const char*, int);
    ConnectionRefusedException(const char*, int, ::Ice::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ConnectionLostException : public ::Ice::SocketException
{
public:

    ConnectionLostException(const char*, int);
    ConnectionLostException(const char*, int, ::Ice::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API DNSException : public ::Ice::LocalException
{
public:

    DNSException(const char*, int);
    DNSException(const char*, int, ::Ice::Int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::Ice::Int error;
    ::std::string host;
};

class ICEE_API TimeoutException : public ::Ice::LocalException
{
public:

    TimeoutException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ConnectTimeoutException : public ::Ice::TimeoutException
{
public:

    ConnectTimeoutException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API CloseTimeoutException : public ::Ice::TimeoutException
{
public:

    CloseTimeoutException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ConnectionTimeoutException : public ::Ice::TimeoutException
{
public:

    ConnectionTimeoutException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ProtocolException : public ::Ice::LocalException
{
public:

    ProtocolException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API BadMagicException : public ::Ice::ProtocolException
{
public:

    BadMagicException(const char*, int);
    BadMagicException(const char*, int, const ::Ice::ByteSeq&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::Ice::ByteSeq badMagic;
};

class ICEE_API UnsupportedProtocolException : public ::Ice::ProtocolException
{
public:

    UnsupportedProtocolException(const char*, int);
    UnsupportedProtocolException(const char*, int, ::Ice::Int, ::Ice::Int, ::Ice::Int, ::Ice::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::Ice::Int badMajor;
    ::Ice::Int badMinor;
    ::Ice::Int major;
    ::Ice::Int minor;
};

class ICEE_API UnsupportedEncodingException : public ::Ice::ProtocolException
{
public:

    UnsupportedEncodingException(const char*, int);
    UnsupportedEncodingException(const char*, int, ::Ice::Int, ::Ice::Int, ::Ice::Int, ::Ice::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::Ice::Int badMajor;
    ::Ice::Int badMinor;
    ::Ice::Int major;
    ::Ice::Int minor;
};

class ICEE_API UnknownMessageException : public ::Ice::ProtocolException
{
public:

    UnknownMessageException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ConnectionNotValidatedException : public ::Ice::ProtocolException
{
public:

    ConnectionNotValidatedException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API UnknownRequestIdException : public ::Ice::ProtocolException
{
public:

    UnknownRequestIdException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API UnknownReplyStatusException : public ::Ice::ProtocolException
{
public:

    UnknownReplyStatusException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API CloseConnectionException : public ::Ice::ProtocolException
{
public:

    CloseConnectionException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ForcedCloseConnectionException : public ::Ice::ProtocolException
{
public:

    ForcedCloseConnectionException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API IllegalMessageSizeException : public ::Ice::ProtocolException
{
public:

    IllegalMessageSizeException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API CompressionNotSupportedException : public ::Ice::ProtocolException
{
public:

    CompressionNotSupportedException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API MarshalException : public ::Ice::ProtocolException
{
public:

    MarshalException(const char*, int);
    MarshalException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string reason;
};

class ICEE_API ProxyUnmarshalException : public ::Ice::MarshalException
{
public:

    ProxyUnmarshalException(const char*, int);
    ProxyUnmarshalException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API UnmarshalOutOfBoundsException : public ::Ice::MarshalException
{
public:

    UnmarshalOutOfBoundsException(const char*, int);
    UnmarshalOutOfBoundsException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API MemoryLimitException : public ::Ice::MarshalException
{
public:

    MemoryLimitException(const char*, int);
    MemoryLimitException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API EncapsulationException : public ::Ice::MarshalException
{
public:

    EncapsulationException(const char*, int);
    EncapsulationException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API NegativeSizeException : public ::Ice::MarshalException
{
public:

    NegativeSizeException(const char*, int);
    NegativeSizeException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

}

#endif
