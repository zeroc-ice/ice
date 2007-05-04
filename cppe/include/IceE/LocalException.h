// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
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

class ICE_API AlreadyRegisteredException : public ::Ice::LocalException
{
public:

    AlreadyRegisteredException(const char*, int);
    AlreadyRegisteredException(const char*, int, const ::std::string&, const ::std::string&);
    virtual ~AlreadyRegisteredException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string kindOfObject;
    ::std::string id;
};

class ICE_API NotRegisteredException : public ::Ice::LocalException
{
public:

    NotRegisteredException(const char*, int);
    NotRegisteredException(const char*, int, const ::std::string&, const ::std::string&);
    virtual ~NotRegisteredException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string kindOfObject;
    ::std::string id;
};

class ICE_API TwowayOnlyException : public ::Ice::LocalException
{
public:

    TwowayOnlyException(const char*, int);
    TwowayOnlyException(const char*, int, const ::std::string&);
    virtual ~TwowayOnlyException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string operation;
};

class ICE_API CloneNotImplementedException : public ::Ice::LocalException
{
public:

    CloneNotImplementedException(const char*, int);
    virtual ~CloneNotImplementedException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API UnknownException : public ::Ice::LocalException
{
public:

    UnknownException(const char*, int);
    UnknownException(const char*, int, const ::std::string&);
    virtual ~UnknownException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string unknown;
};

class ICE_API UnknownLocalException : public ::Ice::UnknownException
{
public:

    UnknownLocalException(const char*, int);
    UnknownLocalException(const char*, int, const ::std::string&);
    virtual ~UnknownLocalException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API UnknownUserException : public ::Ice::UnknownException
{
public:

    UnknownUserException(const char*, int);
    UnknownUserException(const char*, int, const ::std::string&);
    virtual ~UnknownUserException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API VersionMismatchException : public ::Ice::LocalException
{
public:

    VersionMismatchException(const char*, int);
    virtual ~VersionMismatchException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API CommunicatorDestroyedException : public ::Ice::LocalException
{
public:

    CommunicatorDestroyedException(const char*, int);
    virtual ~CommunicatorDestroyedException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

#ifndef ICEE_PURE_CLIENT

class ICE_API ObjectAdapterDeactivatedException : public ::Ice::LocalException
{
public:

    ObjectAdapterDeactivatedException(const char*, int);
    ObjectAdapterDeactivatedException(const char*, int, const ::std::string&);
    virtual ~ObjectAdapterDeactivatedException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string name;
};

class ICE_API ObjectAdapterIdInUseException : public ::Ice::LocalException
{
public:

    ObjectAdapterIdInUseException(const char*, int);
    ObjectAdapterIdInUseException(const char*, int, const ::std::string&);
    virtual ~ObjectAdapterIdInUseException()throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string id;
};
#endif

class ICE_API NoEndpointException : public ::Ice::LocalException
{
public:

    NoEndpointException(const char*, int);
    NoEndpointException(const char*, int, const ::std::string&);
    virtual ~NoEndpointException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string proxy;
};

class ICE_API EndpointParseException : public ::Ice::LocalException
{
public:

    EndpointParseException(const char*, int);
    EndpointParseException(const char*, int, const ::std::string&);
    virtual ~EndpointParseException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string str;
};

class ICE_API IdentityParseException : public ::Ice::LocalException
{
public:

    IdentityParseException(const char*, int);
    IdentityParseException(const char*, int, const ::std::string&);
    virtual ~IdentityParseException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string str;
};

class ICE_API ProxyParseException : public ::Ice::LocalException
{
public:

    ProxyParseException(const char*, int);
    ProxyParseException(const char*, int, const ::std::string&);
    virtual ~ProxyParseException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string str;
};

class ICE_API IllegalIdentityException : public ::Ice::LocalException
{
public:

    IllegalIdentityException(const char*, int);
    IllegalIdentityException(const char*, int, const ::Ice::Identity&);
    virtual ~IllegalIdentityException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::Ice::Identity id;
};

class ICE_API RequestFailedException : public ::Ice::LocalException
{
public:

    RequestFailedException(const char*, int);
    RequestFailedException(const char*, int, const ::Ice::Identity&, const ::std::string&, const ::std::string&);
    virtual ~RequestFailedException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::Ice::Identity id;
    ::std::string facet;
    ::std::string operation;
};

class ICE_API ObjectNotExistException : public ::Ice::RequestFailedException
{
public:

    ObjectNotExistException(const char*, int);
    ObjectNotExistException(const char*, int, const ::Ice::Identity&, const ::std::string&, const ::std::string&);
    virtual ~ObjectNotExistException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API FacetNotExistException : public ::Ice::RequestFailedException
{
public:

    FacetNotExistException(const char*, int);
    FacetNotExistException(const char*, int, const ::Ice::Identity&, const ::std::string&, const ::std::string&);
    virtual ~FacetNotExistException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API OperationNotExistException : public ::Ice::RequestFailedException
{
public:

    OperationNotExistException(const char*, int);
    OperationNotExistException(const char*, int, const ::Ice::Identity&, const ::std::string&, const ::std::string&);
    virtual ~OperationNotExistException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API SyscallException : public ::Ice::LocalException
{
public:

    SyscallException(const char*, int);
    SyscallException(const char*, int, ::Ice::Int);
    virtual ~SyscallException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::Ice::Int error;
};

class ICE_API SocketException : public ::Ice::SyscallException
{
public:

    SocketException(const char*, int);
    SocketException(const char*, int, ::Ice::Int);
    virtual ~SocketException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API FileException : public ::Ice::SyscallException
{
public:

    FileException(const char*, int);
    FileException(const char*, int, ::Ice::Int, const ::std::string&);
    virtual ~FileException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string path;
};

class ICE_API ConnectFailedException : public ::Ice::SocketException
{
public:

    ConnectFailedException(const char*, int);
    ConnectFailedException(const char*, int, ::Ice::Int);
    virtual ~ConnectFailedException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API ConnectionRefusedException : public ::Ice::ConnectFailedException
{
public:

    ConnectionRefusedException(const char*, int);
    ConnectionRefusedException(const char*, int, ::Ice::Int);
    virtual ~ConnectionRefusedException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API ConnectionLostException : public ::Ice::SocketException
{
public:

    ConnectionLostException(const char*, int);
    ConnectionLostException(const char*, int, ::Ice::Int);
    virtual ~ConnectionLostException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API DNSException : public ::Ice::LocalException
{
public:

    DNSException(const char*, int);
    DNSException(const char*, int, ::Ice::Int, const ::std::string&);
    virtual ~DNSException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::Ice::Int error;
    ::std::string host;
};

class ICE_API TimeoutException : public ::Ice::LocalException
{
public:

    TimeoutException(const char*, int);
    virtual ~TimeoutException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API ConnectTimeoutException : public ::Ice::TimeoutException
{
public:

    ConnectTimeoutException(const char*, int);
    virtual ~ConnectTimeoutException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API CloseTimeoutException : public ::Ice::TimeoutException
{
public:

    CloseTimeoutException(const char*, int);
    virtual ~CloseTimeoutException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API ProtocolException : public ::Ice::LocalException
{
public:

    ProtocolException(const char*, int);
    ProtocolException(const char*, int, const ::std::string&);
    virtual ~ProtocolException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string reason;
};

class ICE_API CloseConnectionException : public ::Ice::ProtocolException
{
public:

    CloseConnectionException(const char*, int);
    virtual ~CloseConnectionException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API ForcedCloseConnectionException : public ::Ice::ProtocolException
{
public:

    ForcedCloseConnectionException(const char*, int);
    virtual ~ForcedCloseConnectionException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

ICE_API void throwBadMagicException(const char*, int, const ::Ice::ByteSeq&);
ICE_API void throwUnsupportedProtocolException(const char*, int, ::Ice::Int, ::Ice::Int, ::Ice::Int, ::Ice::Int);
ICE_API void throwUnsupportedEncodingException(const char*, int, ::Ice::Int, ::Ice::Int, ::Ice::Int, ::Ice::Int);

inline void throwUnknownMessageException(const char* file, int line)
{
    throw ProtocolException(file, line, "unknown message type");
}

inline void throwConnectionNotValidatedException(const char* file, int line)
{
    throw ProtocolException(file, line, "received message over unvalidated connection");
}

inline void throwUnknownRequestIdException(const char* file, int line)
{
    throw ProtocolException(file, line, "unknown request id");
}

inline void throwUnknownReplyStatusException(const char* file, int line)
{
    throw ProtocolException(file, line, "unknown reply status");
}

inline void throwIllegalMessageSizeException(const char* file, int line)
{
    throw ProtocolException(file, line, "illegal message size");
}

class ICE_API MarshalException : public ::Ice::ProtocolException
{
public:

    MarshalException(const char*, int);
    MarshalException(const char*, int, const ::std::string&);
    virtual ~MarshalException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_API MemoryLimitException : public ::Ice::MarshalException
{
public:

    MemoryLimitException(const char*, int);
    MemoryLimitException(const char*, int, const ::std::string&);
    virtual ~MemoryLimitException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

ICE_API void throwMemoryLimitException(const char*, int);
ICE_API void throwUnmarshalOutOfBoundsException(const char*, int);
ICE_API void throwNegativeSizeException(const char*, int);

inline void throwProxyUnmarshalException(const char* file, int line)
{     
    throw MarshalException(file, line, "inconsistent proxy data during unmarshaling");
}

class ICE_API FeatureNotSupportedException : public ::Ice::LocalException
{
public:

    FeatureNotSupportedException(const char*, int);
    FeatureNotSupportedException(const char*, int, const std::string&);
    virtual ~FeatureNotSupportedException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string unsupportedFeature;
};

class ICE_API FixedProxyException : public ::Ice::LocalException
{
public:

    FixedProxyException(const char*, int);
    virtual ~FixedProxyException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

#ifdef ICEE_HAS_WSTRING
class ICE_API StringConversionException : public ::Ice::MarshalException
{
public:

    StringConversionException(const char*, int);
    StringConversionException(const char*, int, const ::std::string&); 
    virtual ~StringConversionException() throw();

    virtual ::std::string ice_name() const;
    virtual ::std::string  toString() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;
};
#endif

}

#endif
