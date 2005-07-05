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

namespace IceE
{

class ICEE_API AlreadyRegisteredException : public ::IceE::LocalException
{
public:

    AlreadyRegisteredException(const char*, int);
    AlreadyRegisteredException(const char*, int, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string kindOfObject;
    ::std::string id;
};

class ICEE_API NotRegisteredException : public ::IceE::LocalException
{
public:

    NotRegisteredException(const char*, int);
    NotRegisteredException(const char*, int, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string kindOfObject;
    ::std::string id;
};

class ICEE_API TwowayOnlyException : public ::IceE::LocalException
{
public:

    TwowayOnlyException(const char*, int);
    TwowayOnlyException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string operation;
};

class ICEE_API CloneNotImplementedException : public ::IceE::LocalException
{
public:

    CloneNotImplementedException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API UnknownException : public ::IceE::LocalException
{
public:

    UnknownException(const char*, int);
    UnknownException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string unknown;
};

class ICEE_API UnknownLocalException : public ::IceE::UnknownException
{
public:

    UnknownLocalException(const char*, int);
    UnknownLocalException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API UnknownUserException : public ::IceE::UnknownException
{
public:

    UnknownUserException(const char*, int);
    UnknownUserException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API VersionMismatchException : public ::IceE::LocalException
{
public:

    VersionMismatchException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API CommunicatorDestroyedException : public ::IceE::LocalException
{
public:

    CommunicatorDestroyedException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

#ifndef ICEE_PURE_CLIENT

class ICEE_API ObjectAdapterDeactivatedException : public ::IceE::LocalException
{
public:

    ObjectAdapterDeactivatedException(const char*, int);
    ObjectAdapterDeactivatedException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string name;
};

class ICEE_API ObjectAdapterIdInUseException : public ::IceE::LocalException
{
public:

    ObjectAdapterIdInUseException(const char*, int);
    ObjectAdapterIdInUseException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string id;
};
#endif

class ICEE_API NoEndpointException : public ::IceE::LocalException
{
public:

    NoEndpointException(const char*, int);
    NoEndpointException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string proxy;
};

class ICEE_API EndpointParseException : public ::IceE::LocalException
{
public:

    EndpointParseException(const char*, int);
    EndpointParseException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string str;
};

class ICEE_API IdentityParseException : public ::IceE::LocalException
{
public:

    IdentityParseException(const char*, int);
    IdentityParseException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string str;
};

class ICEE_API ProxyParseException : public ::IceE::LocalException
{
public:

    ProxyParseException(const char*, int);
    ProxyParseException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string str;
};

class ICEE_API IllegalIdentityException : public ::IceE::LocalException
{
public:

    IllegalIdentityException(const char*, int);
    IllegalIdentityException(const char*, int, const ::IceE::Identity&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::IceE::Identity id;
};

class ICEE_API RequestFailedException : public ::IceE::LocalException
{
public:

    RequestFailedException(const char*, int);
    RequestFailedException(const char*, int, const ::IceE::Identity&, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::IceE::Identity id;
    ::std::string facet;
    ::std::string operation;
};

class ICEE_API ObjectNotExistException : public ::IceE::RequestFailedException
{
public:

    ObjectNotExistException(const char*, int);
    ObjectNotExistException(const char*, int, const ::IceE::Identity&, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API FacetNotExistException : public ::IceE::RequestFailedException
{
public:

    FacetNotExistException(const char*, int);
    FacetNotExistException(const char*, int, const ::IceE::Identity&, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API OperationNotExistException : public ::IceE::RequestFailedException
{
public:

    OperationNotExistException(const char*, int);
    OperationNotExistException(const char*, int, const ::IceE::Identity&, const ::std::string&, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API SyscallException : public ::IceE::LocalException
{
public:

    SyscallException(const char*, int);
    SyscallException(const char*, int, ::IceE::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::IceE::Int error;
};

class ICEE_API SocketException : public ::IceE::SyscallException
{
public:

    SocketException(const char*, int);
    SocketException(const char*, int, ::IceE::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API FileException : public ::IceE::SyscallException
{
public:

    FileException(const char*, int);
    FileException(const char*, int, ::IceE::Int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string path;
};

class ICEE_API ConnectFailedException : public ::IceE::SocketException
{
public:

    ConnectFailedException(const char*, int);
    ConnectFailedException(const char*, int, ::IceE::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ConnectionRefusedException : public ::IceE::ConnectFailedException
{
public:

    ConnectionRefusedException(const char*, int);
    ConnectionRefusedException(const char*, int, ::IceE::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ConnectionLostException : public ::IceE::SocketException
{
public:

    ConnectionLostException(const char*, int);
    ConnectionLostException(const char*, int, ::IceE::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API DNSException : public ::IceE::LocalException
{
public:

    DNSException(const char*, int);
    DNSException(const char*, int, ::IceE::Int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::IceE::Int error;
    ::std::string host;
};

class ICEE_API TimeoutException : public ::IceE::LocalException
{
public:

    TimeoutException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ConnectTimeoutException : public ::IceE::TimeoutException
{
public:

    ConnectTimeoutException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API CloseTimeoutException : public ::IceE::TimeoutException
{
public:

    CloseTimeoutException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ConnectionTimeoutException : public ::IceE::TimeoutException
{
public:

    ConnectionTimeoutException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ProtocolException : public ::IceE::LocalException
{
public:

    ProtocolException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API BadMagicException : public ::IceE::ProtocolException
{
public:

    BadMagicException(const char*, int);
    BadMagicException(const char*, int, const ::IceE::ByteSeq&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::IceE::ByteSeq badMagic;
};

class ICEE_API UnsupportedProtocolException : public ::IceE::ProtocolException
{
public:

    UnsupportedProtocolException(const char*, int);
    UnsupportedProtocolException(const char*, int, ::IceE::Int, ::IceE::Int, ::IceE::Int, ::IceE::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::IceE::Int badMajor;
    ::IceE::Int badMinor;
    ::IceE::Int major;
    ::IceE::Int minor;
};

class ICEE_API UnsupportedEncodingException : public ::IceE::ProtocolException
{
public:

    UnsupportedEncodingException(const char*, int);
    UnsupportedEncodingException(const char*, int, ::IceE::Int, ::IceE::Int, ::IceE::Int, ::IceE::Int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::IceE::Int badMajor;
    ::IceE::Int badMinor;
    ::IceE::Int major;
    ::IceE::Int minor;
};

class ICEE_API UnknownMessageException : public ::IceE::ProtocolException
{
public:

    UnknownMessageException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ConnectionNotValidatedException : public ::IceE::ProtocolException
{
public:

    ConnectionNotValidatedException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API UnknownRequestIdException : public ::IceE::ProtocolException
{
public:

    UnknownRequestIdException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API UnknownReplyStatusException : public ::IceE::ProtocolException
{
public:

    UnknownReplyStatusException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API CloseConnectionException : public ::IceE::ProtocolException
{
public:

    CloseConnectionException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API ForcedCloseConnectionException : public ::IceE::ProtocolException
{
public:

    ForcedCloseConnectionException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API IllegalMessageSizeException : public ::IceE::ProtocolException
{
public:

    IllegalMessageSizeException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API CompressionNotSupportedException : public ::IceE::ProtocolException
{
public:

    CompressionNotSupportedException(const char*, int);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API MarshalException : public ::IceE::ProtocolException
{
public:

    MarshalException(const char*, int);
    MarshalException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string reason;
};

class ICEE_API ProxyUnmarshalException : public ::IceE::MarshalException
{
public:

    ProxyUnmarshalException(const char*, int);
    ProxyUnmarshalException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API UnmarshalOutOfBoundsException : public ::IceE::MarshalException
{
public:

    UnmarshalOutOfBoundsException(const char*, int);
    UnmarshalOutOfBoundsException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API MemoryLimitException : public ::IceE::MarshalException
{
public:

    MemoryLimitException(const char*, int);
    MemoryLimitException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API EncapsulationException : public ::IceE::MarshalException
{
public:

    EncapsulationException(const char*, int);
    EncapsulationException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICEE_API NegativeSizeException : public ::IceE::MarshalException
{
public:

    NegativeSizeException(const char*, int);
    NegativeSizeException(const char*, int, const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::IceE::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

}

#endif
