// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/LocalException.h>
#include <Ice/Network.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::UnknownException::UnknownException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::UnknownException::UnknownException(const UnknownException& ex) :
    LocalException(ex)
{
}

UnknownException&
Ice::UnknownException::operator=(const UnknownException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::UnknownException::_name() const
{
    return "Ice::UnknownException";
}

string
Ice::UnknownException::_description() const
{
    return "unknown exception";
}

LocalException*
Ice::UnknownException::_clone() const
{
    return new UnknownException(*this);
}

void
Ice::UnknownException::_throw() const
{
    throw *this;
}

Ice::UnknownUserException::UnknownUserException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::UnknownUserException::UnknownUserException(const UnknownUserException& ex) :
    LocalException(ex)
{
}

UnknownUserException&
Ice::UnknownUserException::operator=(const UnknownUserException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::UnknownUserException::_name() const
{
    return "Ice::UnknownUserException";
}

string
Ice::UnknownUserException::_description() const
{
    return "unknown user exception";
}

LocalException*
Ice::UnknownUserException::_clone() const
{
    return new UnknownUserException(*this);
}

void
Ice::UnknownUserException::_throw() const
{
    throw *this;
}

Ice::VersionMismatchException::VersionMismatchException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::VersionMismatchException::VersionMismatchException(const VersionMismatchException& ex) :
    LocalException(ex)
{
}

VersionMismatchException&
Ice::VersionMismatchException::operator=(const VersionMismatchException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::VersionMismatchException::_name() const
{
    return "Ice::VersionMismatchException";
}

string
Ice::VersionMismatchException::_description() const
{
    return "Ice library version mismatch";
}

LocalException*
Ice::VersionMismatchException::_clone() const
{
    return new VersionMismatchException(*this);
}

void
Ice::VersionMismatchException::_throw() const
{
    throw *this;
}

Ice::CommunicatorDestroyedException::CommunicatorDestroyedException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::CommunicatorDestroyedException::CommunicatorDestroyedException(const CommunicatorDestroyedException& ex) :
    LocalException(ex)
{
}

CommunicatorDestroyedException&
Ice::CommunicatorDestroyedException::operator=(const CommunicatorDestroyedException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::CommunicatorDestroyedException::_name() const
{
    return "Ice::CommunicatorDestroyedException";
}

string
Ice::CommunicatorDestroyedException::_description() const
{
    return "communicator object destroyed";
}

LocalException*
Ice::CommunicatorDestroyedException::_clone() const
{
    return new CommunicatorDestroyedException(*this);
}

void
Ice::CommunicatorDestroyedException::_throw() const
{
    throw *this;
}

Ice::ObjectAdapterDeactivatedException::ObjectAdapterDeactivatedException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::ObjectAdapterDeactivatedException::ObjectAdapterDeactivatedException(const ObjectAdapterDeactivatedException& ex) :
    LocalException(ex)
{
}

ObjectAdapterDeactivatedException&
Ice::ObjectAdapterDeactivatedException::operator=(const ObjectAdapterDeactivatedException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::ObjectAdapterDeactivatedException::_name() const
{
    return "Ice::ObjectAdapterDeactivatedException";
}

string
Ice::ObjectAdapterDeactivatedException::_description() const
{
    return "object adapter deactivated";
}

LocalException*
Ice::ObjectAdapterDeactivatedException::_clone() const
{
    return new ObjectAdapterDeactivatedException(*this);
}

void
Ice::ObjectAdapterDeactivatedException::_throw() const
{
    throw *this;
}

Ice::NoEndpointException::NoEndpointException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::NoEndpointException::NoEndpointException(const NoEndpointException& ex) :
    LocalException(ex)
{
}

NoEndpointException&
Ice::NoEndpointException::operator=(const NoEndpointException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::NoEndpointException::_name() const
{
    return "Ice::NoEndpointException";
}

string
Ice::NoEndpointException::_description() const
{
    return "no suitable endpoint available";
}

LocalException*
Ice::NoEndpointException::_clone() const
{
    return new NoEndpointException(*this);
}

void
Ice::NoEndpointException::_throw() const
{
    throw *this;
}

Ice::EndpointParseException::EndpointParseException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::EndpointParseException::EndpointParseException(const EndpointParseException& ex) :
    LocalException(ex)
{
}

EndpointParseException&
Ice::EndpointParseException::operator=(const EndpointParseException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::EndpointParseException::_name() const
{
    return "Ice::EndpointParseException";
}

string
Ice::EndpointParseException::_description() const
{
    return "error while parsing endpoint";
}

LocalException*
Ice::EndpointParseException::_clone() const
{
    return new EndpointParseException(*this);
}

void
Ice::EndpointParseException::_throw() const
{
    throw *this;
}

Ice::ReferenceParseException::ReferenceParseException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::ReferenceParseException::ReferenceParseException(const ReferenceParseException& ex) :
    LocalException(ex)
{
}

ReferenceParseException&
Ice::ReferenceParseException::operator=(const ReferenceParseException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::ReferenceParseException::_name() const
{
    return "Ice::ReferenceParseException";
}

string
Ice::ReferenceParseException::_description() const
{
    return "error while parsing reference";
}

LocalException*
Ice::ReferenceParseException::_clone() const
{
    return new ReferenceParseException(*this);
}

void
Ice::ReferenceParseException::_throw() const
{
    throw *this;
}

Ice::ReferenceIdentityException::ReferenceIdentityException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::ReferenceIdentityException::ReferenceIdentityException(const ReferenceIdentityException& ex) :
    LocalException(ex)
{
}

ReferenceIdentityException&
Ice::ReferenceIdentityException::operator=(const ReferenceIdentityException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::ReferenceIdentityException::_name() const
{
    return "Ice::ReferenceIdentityException";
}

string
Ice::ReferenceIdentityException::_description() const
{
    return "reference identity mismatch in location forward";
}

LocalException*
Ice::ReferenceIdentityException::_clone() const
{
    return new ReferenceIdentityException(*this);
}

void
Ice::ReferenceIdentityException::_throw() const
{
    throw *this;
}

Ice::ObjectNotExistException::ObjectNotExistException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::ObjectNotExistException::ObjectNotExistException(const ObjectNotExistException& ex) :
    LocalException(ex)
{
}

ObjectNotExistException&
Ice::ObjectNotExistException::operator=(const ObjectNotExistException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::ObjectNotExistException::_name() const
{
    return "Ice::ObjectNotExistException";
}

string
Ice::ObjectNotExistException::_description() const
{
    return "object does not exist";
}

LocalException*
Ice::ObjectNotExistException::_clone() const
{
    return new ObjectNotExistException(*this);
}

void
Ice::ObjectNotExistException::_throw() const
{
    throw *this;
}

Ice::OperationNotExistException::OperationNotExistException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::OperationNotExistException::OperationNotExistException(const OperationNotExistException& ex) :
    LocalException(ex)
{
}

OperationNotExistException&
Ice::OperationNotExistException::operator=(const OperationNotExistException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::OperationNotExistException::_name() const
{
    return "Ice::OperationNotExistException";
}

string
Ice::OperationNotExistException::_description() const
{
    return "operation does not exist";
}

LocalException*
Ice::OperationNotExistException::_clone() const
{
    return new OperationNotExistException(*this);
}

void
Ice::OperationNotExistException::_throw() const
{
    throw *this;
}

Ice::NoServantFactoryException::NoServantFactoryException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::NoServantFactoryException::NoServantFactoryException(const NoServantFactoryException& ex) :
    LocalException(ex)
{
}

NoServantFactoryException&
Ice::NoServantFactoryException::operator=(const NoServantFactoryException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::NoServantFactoryException::_name() const
{
    return "Ice::NoServantFactoryException";
}

string
Ice::NoServantFactoryException::_description() const
{
    return "no servant factory found for the requested servant type";
}

LocalException*
Ice::NoServantFactoryException::_clone() const
{
    return new NoServantFactoryException(*this);
}

void
Ice::NoServantFactoryException::_throw() const
{
    throw *this;
}

Ice::SystemException::SystemException(const char* file, int line) :
    LocalException(file, line)
{
#ifdef WIN32
    _error = GetLastError();
#else
    _error = errno;
#endif
}

Ice::SystemException::SystemException(const SystemException& ex) :
    LocalException(ex)
{
    _error = ex._error;
}

SystemException&
Ice::SystemException::operator=(const SystemException& ex)
{
    LocalException::operator=(ex);

    if (this != &ex)
    {
	_error = ex._error;
    }

    return *this;
}

string
Ice::SystemException::_name() const
{
    return "Ice::SystemException";
}
string
Ice::SystemException::_description() const
{
    return "system exception: " + errorToString(_error);
}

LocalException*
Ice::SystemException::_clone() const
{
    return new SystemException(*this);
}

void
Ice::SystemException::_throw() const
{
    throw *this;
}

Ice::SocketException::SocketException(const char* file, int line) :
    SystemException(file, line)
{
#ifdef WIN32
    //
    // Overwrite _error, which has been set by GetLastError() in the
    // SystemException constructor, with WSAGetLastError()
    //
    _error = WSAGetLastError();
#endif
}

Ice::SocketException::SocketException(const SocketException& ex) :
    SystemException(ex)
{
}

SocketException&
Ice::SocketException::operator=(const SocketException& ex)
{
    SystemException::operator=(ex);
    return *this;
}

string
Ice::SocketException::_name() const
{
    return "Ice::SocketException";
}

string
Ice::SocketException::_description() const
{
    return "system exception: " + errorToString(_error);
}

LocalException*
Ice::SocketException::_clone() const
{
    return new SocketException(*this);
}

void
Ice::SocketException::_throw() const
{
    throw *this;
}

Ice::TimeoutException::TimeoutException(const char* file, int line) :
    SocketException(file, line)
{
}

Ice::TimeoutException::TimeoutException(const TimeoutException& ex) :
    SocketException(ex)
{
}

TimeoutException&
Ice::TimeoutException::operator=(const TimeoutException& ex)
{
    SocketException::operator=(ex);
    return *this;
}

string
Ice::TimeoutException::_name() const
{
    return "Ice::TimeoutException";
}

string
Ice::TimeoutException::_description() const
{
    return "timeout while sending or receiving data";
}

LocalException*
Ice::TimeoutException::_clone() const
{
    return new TimeoutException(*this);
}

void
Ice::TimeoutException::_throw() const
{
    throw *this;
}

Ice::ConnectTimeoutException::ConnectTimeoutException(const char* file, int line) :
    TimeoutException(file, line)
{
}

Ice::ConnectTimeoutException::ConnectTimeoutException(const ConnectTimeoutException& ex) :
    TimeoutException(ex)
{
}

ConnectTimeoutException&
Ice::ConnectTimeoutException::operator=(const ConnectTimeoutException& ex)
{
    TimeoutException::operator=(ex);
    return *this;
}

string
Ice::ConnectTimeoutException::_name() const
{
    return "Ice::ConnectTimeoutException";
}

string
Ice::ConnectTimeoutException::_description() const
{
    return "timeout while establishing a connection";
}

LocalException*
Ice::ConnectTimeoutException::_clone() const
{
    return new ConnectTimeoutException(*this);
}

void
Ice::ConnectTimeoutException::_throw() const
{
    throw *this;
}

Ice::ConnectFailedException::ConnectFailedException(const char* file, int line) :
    SocketException(file, line)
{
}

Ice::ConnectFailedException::ConnectFailedException(const ConnectFailedException& ex) :
    SocketException(ex)
{
}

ConnectFailedException&
Ice::ConnectFailedException::operator=(const ConnectFailedException& ex)
{
    SocketException::operator=(ex);
    return *this;
}

string
Ice::ConnectFailedException::_name() const
{
    return "Ice::ConnectFailedException";
}

string
Ice::ConnectFailedException::_description() const
{
    return "connect failed: " + errorToString(_error);
}

LocalException*
Ice::ConnectFailedException::_clone() const
{
    return new ConnectFailedException(*this);
}

void
Ice::ConnectFailedException::_throw() const
{
    throw *this;
}

Ice::ConnectionLostException::ConnectionLostException(const char* file, int line) :
    SocketException(file, line)
{
}

Ice::ConnectionLostException::ConnectionLostException(const ConnectionLostException& ex) :
    SocketException(ex)
{
}

ConnectionLostException&
Ice::ConnectionLostException::operator=(const ConnectionLostException& ex)
{
    SocketException::operator=(ex);
    return *this;
}

string
Ice::ConnectionLostException::_name() const
{
    return "Ice::ConnectionLostException";
}

string
Ice::ConnectionLostException::_description() const
{
    string s = "connection lost: ";
    if (_error == 0)
    {
	s += "recv() returned zero";
    }
    else
    {
	s += errorToString(_error);
    }
    return s;
}

LocalException*
Ice::ConnectionLostException::_clone() const
{
    return new ConnectionLostException(*this);
}

void
Ice::ConnectionLostException::_throw() const
{
    throw *this;
}

Ice::DNSException::DNSException(const char* file, int line) :
    SystemException(file, line)
{
#ifndef WIN32
    _error = h_errno;
#endif
}

Ice::DNSException::DNSException(const DNSException& ex) :
    SystemException(ex)
{
}

DNSException&
Ice::DNSException::operator=(const DNSException& ex)
{
    SystemException::operator=(ex);
    return *this;
}

string
Ice::DNSException::_name() const
{
    return "Ice::DNSException";
}

string
Ice::DNSException::_description() const
{
    return "DNS error: " + errorToStringDNS(_error);
}

LocalException*
Ice::DNSException::_clone() const
{
    return new DNSException(*this);
}

void
Ice::DNSException::_throw() const
{
    throw *this;
}

Ice::ProtocolException::ProtocolException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::ProtocolException::ProtocolException(const ProtocolException& ex) :
    LocalException(ex)
{
}

ProtocolException&
Ice::ProtocolException::operator=(const ProtocolException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

Ice::UnmarshalOutOfBoundsException::UnmarshalOutOfBoundsException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::UnmarshalOutOfBoundsException::UnmarshalOutOfBoundsException(const UnmarshalOutOfBoundsException& ex) :
    ProtocolException(ex)
{
}

UnmarshalOutOfBoundsException&
Ice::UnmarshalOutOfBoundsException::operator=(const UnmarshalOutOfBoundsException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::UnmarshalOutOfBoundsException::_name() const
{
    return "Ice::UnmarshalOutOfBoundsException";
}

string
Ice::UnmarshalOutOfBoundsException::_description() const
{
    return "protocol error: out of bounds during unmarshaling";
}

LocalException*
Ice::UnmarshalOutOfBoundsException::_clone() const
{
    return new UnmarshalOutOfBoundsException(*this);
}

void
Ice::UnmarshalOutOfBoundsException::_throw() const
{
    throw *this;
}

Ice::ServantUnmarshalException::ServantUnmarshalException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::ServantUnmarshalException::ServantUnmarshalException(const ServantUnmarshalException& ex) :
    ProtocolException(ex)
{
}

ServantUnmarshalException&
Ice::ServantUnmarshalException::operator=(const ServantUnmarshalException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::ServantUnmarshalException::_name() const
{
    return "Ice::ServantUnmarshalException";
}

string
Ice::ServantUnmarshalException::_description() const
{
    return "protocol error: servant type does not match signature";
}

LocalException*
Ice::ServantUnmarshalException::_clone() const
{
    return new ServantUnmarshalException(*this);
}

void
Ice::ServantUnmarshalException::_throw() const
{
    throw *this;
}

Ice::StringEncodingException::StringEncodingException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::StringEncodingException::StringEncodingException(const StringEncodingException& ex) :
    ProtocolException(ex)
{
}

StringEncodingException&
Ice::StringEncodingException::operator=(const StringEncodingException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::StringEncodingException::_name() const
{
    return "Ice::StringEncodingException";
}

string
Ice::StringEncodingException::_description() const
{
    return "protocol error: string encoding error";
}

LocalException*
Ice::StringEncodingException::_clone() const
{
    return new StringEncodingException(*this);
}

void
Ice::StringEncodingException::_throw() const
{
    throw *this;
}

Ice::MemoryLimitException::MemoryLimitException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::MemoryLimitException::MemoryLimitException(const MemoryLimitException& ex) :
    ProtocolException(ex)
{
}

MemoryLimitException&
Ice::MemoryLimitException::operator=(const MemoryLimitException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::MemoryLimitException::_name() const
{
    return "Ice::MemoryLimitException";
}

string
Ice::MemoryLimitException::_description() const
{
    return "protocol error: memory limit exceeded";
}

LocalException*
Ice::MemoryLimitException::_clone() const
{
    return new MemoryLimitException(*this);
}

void
Ice::MemoryLimitException::_throw() const
{
    throw *this;
}

Ice::EncapsulationException::EncapsulationException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::EncapsulationException::EncapsulationException(const EncapsulationException& ex) :
    ProtocolException(ex)
{
}

EncapsulationException&
Ice::EncapsulationException::operator=(const EncapsulationException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::EncapsulationException::_name() const
{
    return "Ice::EncapsulationException";
}

string
Ice::EncapsulationException::_description() const
{
    return "protocol error: illegal encapsulation";
}

LocalException*
Ice::EncapsulationException::_clone() const
{
    return new EncapsulationException(*this);
}

void
Ice::EncapsulationException::_throw() const
{
    throw *this;
}

Ice::UnsupportedProtocolException::UnsupportedProtocolException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::UnsupportedProtocolException::UnsupportedProtocolException(const UnsupportedProtocolException& ex) :
    ProtocolException(ex)
{
}

UnsupportedProtocolException&
Ice::UnsupportedProtocolException::operator=(const UnsupportedProtocolException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::UnsupportedProtocolException::_name() const
{
    return "Ice::UnsupportedProtocolException";
}

string
Ice::UnsupportedProtocolException::_description() const
{
    return "protocol error: unsupported protocol version";
}

LocalException*
Ice::UnsupportedProtocolException::_clone() const
{
    return new UnsupportedProtocolException(*this);
}

void
Ice::UnsupportedProtocolException::_throw() const
{
    throw *this;
}

Ice::UnsupportedEncodingException::UnsupportedEncodingException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::UnsupportedEncodingException::UnsupportedEncodingException(const UnsupportedEncodingException& ex) :
    ProtocolException(ex)
{
}

UnsupportedEncodingException&
Ice::UnsupportedEncodingException::operator=(const UnsupportedEncodingException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::UnsupportedEncodingException::_name() const
{
    return "Ice::UnsupportedEncodingException";
}

string
Ice::UnsupportedEncodingException::_description() const
{
    return "protocol error: unsupported encoding version";
}

LocalException*
Ice::UnsupportedEncodingException::_clone() const
{
    return new UnsupportedEncodingException(*this);
}

void
Ice::UnsupportedEncodingException::_throw() const
{
    throw *this;
}

Ice::InvalidMessageException::InvalidMessageException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::InvalidMessageException::InvalidMessageException(const InvalidMessageException& ex) :
    ProtocolException(ex)
{
}

InvalidMessageException&
Ice::InvalidMessageException::operator=(const InvalidMessageException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::InvalidMessageException::_name() const
{
    return "Ice::InvalidMessageException";
}

string
Ice::InvalidMessageException::_description() const
{
    return "protocol error: invalid message type";
}

LocalException*
Ice::InvalidMessageException::_clone() const
{
    return new InvalidMessageException(*this);
}

void
Ice::InvalidMessageException::_throw() const
{
    throw *this;
}

Ice::UnknownMessageException::UnknownMessageException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::UnknownMessageException::UnknownMessageException(const UnknownMessageException& ex) :
    ProtocolException(ex)
{
}

UnknownMessageException&
Ice::UnknownMessageException::operator=(const UnknownMessageException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::UnknownMessageException::_name() const
{
    return "Ice::UnknownMessageException";
}

string
Ice::UnknownMessageException::_description() const
{
    return "protocol error: unknown message type";
}

LocalException*
Ice::UnknownMessageException::_clone() const
{
    return new UnknownMessageException(*this);
}

void
Ice::UnknownMessageException::_throw() const
{
    throw *this;
}

Ice::UnknownRequestIdException::UnknownRequestIdException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::UnknownRequestIdException::UnknownRequestIdException(const UnknownRequestIdException& ex) :
    ProtocolException(ex)
{
}

UnknownRequestIdException&
Ice::UnknownRequestIdException::operator=(const UnknownRequestIdException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::UnknownRequestIdException::_name() const
{
    return "Ice::UnknownRequestIdException";
}

string
Ice::UnknownRequestIdException::_description() const
{
    return "protocol error: unknown request id";
}

LocalException*
Ice::UnknownRequestIdException::_clone() const
{
    return new UnknownRequestIdException(*this);
}

void
Ice::UnknownRequestIdException::_throw() const
{
    throw *this;
}

Ice::UnknownReplyStatusException::UnknownReplyStatusException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::UnknownReplyStatusException::UnknownReplyStatusException(const UnknownReplyStatusException& ex) :
    ProtocolException(ex)
{
}

UnknownReplyStatusException&
Ice::UnknownReplyStatusException::operator=(const UnknownReplyStatusException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::UnknownReplyStatusException::_name() const
{
    return "Ice::UnknownReplyStatusException";
}

string
Ice::UnknownReplyStatusException::_description() const
{
    return "protocol error: unknown reply status";
}

LocalException*
Ice::UnknownReplyStatusException::_clone() const
{
    return new UnknownReplyStatusException(*this);
}

void
Ice::UnknownReplyStatusException::_throw() const
{
    throw *this;
}

Ice::CloseConnectionException::CloseConnectionException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::CloseConnectionException::CloseConnectionException(const CloseConnectionException& ex) :
    ProtocolException(ex)
{
}

CloseConnectionException&
Ice::CloseConnectionException::operator=(const CloseConnectionException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::CloseConnectionException::_name() const
{
    return "Ice::CloseConnectionException";
}

string
Ice::CloseConnectionException::_description() const
{
    return "protocol error: connection closed by server";
}

LocalException*
Ice::CloseConnectionException::_clone() const
{
    return new CloseConnectionException(*this);
}

void
Ice::CloseConnectionException::_throw() const
{
    throw *this;
}

Ice::AbortBatchRequestException::AbortBatchRequestException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::AbortBatchRequestException::AbortBatchRequestException(const AbortBatchRequestException& ex) :
    ProtocolException(ex)
{
}

AbortBatchRequestException&
Ice::AbortBatchRequestException::operator=(const AbortBatchRequestException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::AbortBatchRequestException::_name() const
{
    return "Ice::AbortBatchRequestException";
}

string
Ice::AbortBatchRequestException::_description() const
{
    return "protocol error: batch request was aborted";
}

LocalException*
Ice::AbortBatchRequestException::_clone() const
{
    return new AbortBatchRequestException(*this);
}

void
Ice::AbortBatchRequestException::_throw() const
{
    throw *this;
}
