// **********************************************************************
//
// Copyright (c) 2001
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

    LocalException(const char*, int);
    virtual ~LocalException();
    LocalException(const LocalException&);
    LocalException& operator=(const LocalException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;

protected:

    std::string debugInfo() const;

private:

    const char* _file;
    int _line;
};

ICE_API std::ostream& operator<<(std::ostream&, const LocalException&);

class ICE_API UnknownException : public LocalException
{
public:    

    UnknownException(const char*, int);
    UnknownException(const UnknownException&);
    UnknownException& operator=(const UnknownException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API UnknownUserException : public LocalException
{
public:    

    UnknownUserException(const char*, int);
    UnknownUserException(const UnknownUserException&);
    UnknownUserException& operator=(const UnknownUserException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API VersionMismatchException : public LocalException
{
public:    

    VersionMismatchException(const char*, int);
    VersionMismatchException(const VersionMismatchException&);
    VersionMismatchException& operator=(const VersionMismatchException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API CommunicatorDestroyedException : public LocalException
{
public:    

    CommunicatorDestroyedException(const char*, int);
    CommunicatorDestroyedException(const CommunicatorDestroyedException&);
    CommunicatorDestroyedException& operator=(
	const CommunicatorDestroyedException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API ObjectAdapterDeactivatedException : public LocalException
{
public:    

    ObjectAdapterDeactivatedException(const char*, int);
    ObjectAdapterDeactivatedException(
	const ObjectAdapterDeactivatedException&);
    ObjectAdapterDeactivatedException& operator=(
	const ObjectAdapterDeactivatedException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API NoEndpointException : public LocalException
{
public:    

    NoEndpointException(const char*, int);
    NoEndpointException(const NoEndpointException&);
    NoEndpointException& operator=(const NoEndpointException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API EndpointParseException : public LocalException
{
public:    

    EndpointParseException(const char*, int);
    EndpointParseException(const EndpointParseException&);
    EndpointParseException& operator=(const EndpointParseException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API ReferenceParseException : public LocalException
{
public:    

    ReferenceParseException(const char*, int);
    ReferenceParseException(const ReferenceParseException&);
    ReferenceParseException& operator=(const ReferenceParseException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API ReferenceIdentityException : public LocalException
{
public:    

    ReferenceIdentityException(const char*, int);
    ReferenceIdentityException(const ReferenceIdentityException&);
    ReferenceIdentityException& operator=(const ReferenceIdentityException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API ObjectNotExistException : public LocalException
{
public:    

    ObjectNotExistException(const char*, int);
    ObjectNotExistException(const ObjectNotExistException&);
    ObjectNotExistException& operator=(const ObjectNotExistException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API OperationNotExistException : public LocalException
{
public:    

    OperationNotExistException(const char*, int);
    OperationNotExistException(const OperationNotExistException&);
    OperationNotExistException& operator=(const OperationNotExistException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API NoServantFactoryException : public LocalException
{
public:    

    NoServantFactoryException(const char*, int);
    NoServantFactoryException(const NoServantFactoryException&);
    NoServantFactoryException& operator=(const NoServantFactoryException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API SystemException : public LocalException
{
public:    

    SystemException(const char*, int);
    SystemException(const SystemException&);
    SystemException& operator=(const SystemException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;

protected:

    int _error;
};

class ICE_API SocketException : public SystemException
{
public:    

    SocketException(const char*, int);
    SocketException(const SocketException&);
    SocketException& operator=(const SocketException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API TimeoutException : public SocketException
{
public:    

    TimeoutException(const char*, int);
    TimeoutException(const TimeoutException&);
    TimeoutException& operator=(const TimeoutException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API ConnectTimeoutException : public TimeoutException
{
public:    

    ConnectTimeoutException(const char*, int);
    ConnectTimeoutException(const ConnectTimeoutException&);
    ConnectTimeoutException& operator=(const ConnectTimeoutException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API ConnectFailedException : public SocketException
{
public:    

    ConnectFailedException(const char*, int);
    ConnectFailedException(const ConnectFailedException&);
    ConnectFailedException& operator=(const ConnectFailedException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API ConnectionLostException : public SocketException
{
public:    

    ConnectionLostException(const char*, int);
    ConnectionLostException(const ConnectionLostException&);
    ConnectionLostException& operator=(const ConnectionLostException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API DNSException : public SystemException
{
public:    

    DNSException(const char*, int);
    DNSException(const DNSException&);
    DNSException& operator=(const DNSException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API ProtocolException : public LocalException
{
public:

    ProtocolException(const char*, int);
    ProtocolException(const ProtocolException&);
    ProtocolException& operator=(const ProtocolException&);
};

class ICE_API UnmarshalOutOfBoundsException : public ProtocolException
{
public:    
    
    UnmarshalOutOfBoundsException(const char*, int);
    UnmarshalOutOfBoundsException(const UnmarshalOutOfBoundsException&);
    UnmarshalOutOfBoundsException& operator=(
	const UnmarshalOutOfBoundsException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API ServantUnmarshalException : public ProtocolException
{
public:    
    
    ServantUnmarshalException(const char*, int);
    ServantUnmarshalException(const ServantUnmarshalException&);
    ServantUnmarshalException& operator=(const ServantUnmarshalException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API StringEncodingException : public ProtocolException
{
public:    
    
    StringEncodingException(const char*, int);
    StringEncodingException(const StringEncodingException&);
    StringEncodingException& operator=(const StringEncodingException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API MemoryLimitException : public ProtocolException
{
public:    
    
    MemoryLimitException(const char*, int);
    MemoryLimitException(const MemoryLimitException&);
    MemoryLimitException& operator=(const MemoryLimitException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API EncapsulationException : public ProtocolException
{
public:    
    
    EncapsulationException(const char*, int);
    EncapsulationException(const EncapsulationException&);
    EncapsulationException& operator=(const EncapsulationException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API UnsupportedProtocolException : public ProtocolException
{
public:    

    UnsupportedProtocolException(const char*, int);
    UnsupportedProtocolException(const UnsupportedProtocolException&);
    UnsupportedProtocolException& operator=(
	const UnsupportedProtocolException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API UnsupportedEncodingException : public ProtocolException
{
public:    

    UnsupportedEncodingException(const char*, int);
    UnsupportedEncodingException(const UnsupportedEncodingException&);
    UnsupportedEncodingException& operator=(
	const UnsupportedEncodingException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API InvalidMessageException : public ProtocolException
{
public:    

    InvalidMessageException(const char*, int);
    InvalidMessageException(const InvalidMessageException&);
    InvalidMessageException& operator=(const InvalidMessageException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API UnknownMessageException : public ProtocolException
{
public:    

    UnknownMessageException(const char*, int);
    UnknownMessageException(const UnknownMessageException&);
    UnknownMessageException& operator=(const UnknownMessageException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API UnknownRequestIdException : public ProtocolException
{
public:    

    UnknownRequestIdException(const char*, int);
    UnknownRequestIdException(const UnknownRequestIdException&);
    UnknownRequestIdException& operator=(const UnknownRequestIdException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API UnknownReplyStatusException : public ProtocolException
{
public:    

    UnknownReplyStatusException(const char*, int);
    UnknownReplyStatusException(const UnknownReplyStatusException&);
    UnknownReplyStatusException& operator=(const UnknownReplyStatusException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API CloseConnectionException : public ProtocolException
{
public:    

    CloseConnectionException(const char*, int);
    CloseConnectionException(const CloseConnectionException&);
    CloseConnectionException& operator=(const CloseConnectionException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

class ICE_API AbortBatchRequestException : public ProtocolException
{
public:    

    AbortBatchRequestException(const char*, int);
    AbortBatchRequestException(const AbortBatchRequestException&);
    AbortBatchRequestException& operator=(const AbortBatchRequestException&);
    virtual std::string toString() const;
    virtual LocalException* clone() const;
    virtual void raise() const;
};

}

#endif
