// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_EXCEPTION_H
#define ICE_EXCEPTION_H

#include <IceUtil/Exception.h>
#include <Ice/Config.h>

namespace Ice
{

typedef IceUtil::Exception Exception;

class ICE_API LocalException : public IceUtil::Exception
{
public:    

    LocalException(const char*, int);
    LocalException(const LocalException&);
    LocalException& operator=(const LocalException&);
    virtual std::string _name() const;
    virtual std::ostream& _print(std::ostream&) const;
    virtual Exception* _clone() const;
    virtual void _throw() const;
};

class ICE_API UserException : public IceUtil::Exception
{
public:    

    UserException();
    UserException(const UserException&);
    UserException& operator=(const UserException&);
    virtual std::string _name() const;
    virtual std::ostream& _print(std::ostream&) const;
    virtual Exception* _clone() const;
    virtual void _throw() const;
};

class UnknownException;
class VersionMismatchException;
class CommunicatorDestroyedException;
class ObjectAdapterDeactivatedException;
class NoEndpointException;
class EndpointParseException;
class ReferenceParseException;
class ReferenceIdentityException;
class ObjectNotExistException;
class OperationNotExistException;
class SystemException;
class SocketException;
class TimeoutException;
class ConnectTimeoutException;
class ConnectFailedException;
class ConnectionLostException;
class DNSException;
class ProtocolException;
class UnmarshalOutOfBoundsException;
class ServantUnmarshalException;
class StringEncodingException;
class MemoryLimitException;
class EncapsulationException;
class UnsupportedProtocolException;
class UnsupportedEncodingException;
class InvalidMessageException;
class UnknownMessageException;
class UnknownRequestIdException;
class UnknownReplyStatusException;
class CloseConnectionException;
class AbortBatchRequestException;

}

namespace IceUtil
{

std::ostream& printException(std::ostream&, const Ice::LocalException&);
std::ostream& printException(std::ostream&, const Ice::UserException&);
std::ostream& printException(std::ostream&, const Ice::UnknownException&);
std::ostream& printException(std::ostream&, const Ice::VersionMismatchException&);
std::ostream& printException(std::ostream&, const Ice::CommunicatorDestroyedException&);
std::ostream& printException(std::ostream&, const Ice::ObjectAdapterDeactivatedException&);
std::ostream& printException(std::ostream&, const Ice::NoEndpointException&);
std::ostream& printException(std::ostream&, const Ice::EndpointParseException&);
std::ostream& printException(std::ostream&, const Ice::ReferenceParseException&);
std::ostream& printException(std::ostream&, const Ice::ReferenceIdentityException&);
std::ostream& printException(std::ostream&, const Ice::ObjectNotExistException&);
std::ostream& printException(std::ostream&, const Ice::OperationNotExistException&);
std::ostream& printException(std::ostream&, const Ice::SystemException&);
std::ostream& printException(std::ostream&, const Ice::SocketException&);
std::ostream& printException(std::ostream&, const Ice::TimeoutException&);
std::ostream& printException(std::ostream&, const Ice::ConnectTimeoutException&);
std::ostream& printException(std::ostream&, const Ice::ConnectFailedException&);
std::ostream& printException(std::ostream&, const Ice::ConnectionLostException&);
std::ostream& printException(std::ostream&, const Ice::DNSException&);
std::ostream& printException(std::ostream&, const Ice::ProtocolException&);
std::ostream& printException(std::ostream&, const Ice::UnmarshalOutOfBoundsException&);
std::ostream& printException(std::ostream&, const Ice::ServantUnmarshalException&);
std::ostream& printException(std::ostream&, const Ice::StringEncodingException&);
std::ostream& printException(std::ostream&, const Ice::MemoryLimitException&);
std::ostream& printException(std::ostream&, const Ice::EncapsulationException&);
std::ostream& printException(std::ostream&, const Ice::UnsupportedProtocolException&);
std::ostream& printException(std::ostream&, const Ice::UnsupportedEncodingException&);
std::ostream& printException(std::ostream&, const Ice::InvalidMessageException&);
std::ostream& printException(std::ostream&, const Ice::UnknownMessageException&);
std::ostream& printException(std::ostream&, const Ice::UnknownRequestIdException&);
std::ostream& printException(std::ostream&, const Ice::UnknownReplyStatusException&);
std::ostream& printException(std::ostream&, const Ice::CloseConnectionException&);
std::ostream& printException(std::ostream&, const Ice::AbortBatchRequestException&);

}

#include <Ice/LocalException.h>

#endif
