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
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::LocalException::LocalException(const char* file, int line) :
    _file(file),
    _line(line)
{
}

Ice::LocalException::~LocalException()
{
}

Ice::LocalException::LocalException(const LocalException& ex)
{
    _line = ex._line;
    _file = ex._file;
}

LocalException&
Ice::LocalException::operator=(const LocalException& ex)
{
    if (this != &ex)
    {
	_line = ex._line;
	_file = ex._file;
    }

    return *this;
}

string
Ice::LocalException::toString() const
{
    return debugInfo() + "unknown local exception";
}

LocalException*
Ice::LocalException::clone() const
{
    return new LocalException(*this);
}

void
Ice::LocalException::raise() const
{
    throw *this;
}

std::string
Ice::LocalException::debugInfo() const
{
    ostringstream s;
    s << _file << ':' << _line << ": ";
    return s.str();
}

ostream&
Ice::operator<<(ostream& out, const LocalException& ex)
{
    string s = ex.toString();
    return out << s;
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
Ice::UnknownUserException::toString() const
{
    string s = debugInfo() + "unknown user exception";
    return s;
}

LocalException*
Ice::UnknownUserException::clone() const
{
    return new UnknownUserException(*this);
}

void
Ice::UnknownUserException::raise() const
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
Ice::VersionMismatchException::toString() const
{
    string s = debugInfo() + "Ice library version mismatch";
    return s;
}

LocalException*
Ice::VersionMismatchException::clone() const
{
    return new VersionMismatchException(*this);
}

void
Ice::VersionMismatchException::raise() const
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
Ice::CommunicatorDestroyedException::toString() const
{
    string s = debugInfo() + "communicator object destroyed";
    return s;
}

LocalException*
Ice::CommunicatorDestroyedException::clone() const
{
    return new CommunicatorDestroyedException(*this);
}

void
Ice::CommunicatorDestroyedException::raise() const
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
Ice::ObjectAdapterDeactivatedException::toString() const
{
    string s = debugInfo() + "object adapter deactivated";
    return s;
}

LocalException*
Ice::ObjectAdapterDeactivatedException::clone() const
{
    return new ObjectAdapterDeactivatedException(*this);
}

void
Ice::ObjectAdapterDeactivatedException::raise() const
{
    throw *this;
}

Ice::WrongObjectAdapterException::WrongObjectAdapterException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::WrongObjectAdapterException::WrongObjectAdapterException(const WrongObjectAdapterException& ex) :
    LocalException(ex)
{
}

WrongObjectAdapterException&
Ice::WrongObjectAdapterException::operator=(const WrongObjectAdapterException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::WrongObjectAdapterException::toString() const
{
    string s = debugInfo() + "no suitable endpoint information available";
    return s;
}

LocalException*
Ice::WrongObjectAdapterException::clone() const
{
    return new WrongObjectAdapterException(*this);
}

void
Ice::WrongObjectAdapterException::raise() const
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
Ice::NoEndpointException::toString() const
{
    string s = debugInfo() + "no suitable endpoint information available";
    return s;
}

LocalException*
Ice::NoEndpointException::clone() const
{
    return new NoEndpointException(*this);
}

void
Ice::NoEndpointException::raise() const
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
Ice::EndpointParseException::toString() const
{
    string s = debugInfo() + "error while parsing endpoint";
    return s;
}

LocalException*
Ice::EndpointParseException::clone() const
{
    return new EndpointParseException(*this);
}

void
Ice::EndpointParseException::raise() const
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
Ice::ReferenceParseException::toString() const
{
    string s = debugInfo() + "error while parsing reference";
    return s;
}

LocalException*
Ice::ReferenceParseException::clone() const
{
    return new ReferenceParseException(*this);
}

void
Ice::ReferenceParseException::raise() const
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
Ice::ReferenceIdentityException::toString() const
{
    string s = debugInfo() + "reference identity mismatch in location forward";
    return s;
}

LocalException*
Ice::ReferenceIdentityException::clone() const
{
    return new ReferenceIdentityException(*this);
}

void
Ice::ReferenceIdentityException::raise() const
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
Ice::ObjectNotExistException::toString() const
{
    string s = debugInfo() + "object does not exist";
    return s;
}

LocalException*
Ice::ObjectNotExistException::clone() const
{
    return new ObjectNotExistException(*this);
}

void
Ice::ObjectNotExistException::raise() const
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
Ice::OperationNotExistException::toString() const
{
    string s = debugInfo() + "operation does not exist";
    return s;
}

LocalException*
Ice::OperationNotExistException::clone() const
{
    return new OperationNotExistException(*this);
}

void
Ice::OperationNotExistException::raise() const
{
    throw *this;
}

Ice::NoFactoryException::NoFactoryException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::NoFactoryException::NoFactoryException(const NoFactoryException& ex) :
    LocalException(ex)
{
}

NoFactoryException&
Ice::NoFactoryException::operator=(const NoFactoryException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
Ice::NoFactoryException::toString() const
{
    string s = debugInfo() + "no factory installed for class with operations";
    return s;
}

LocalException*
Ice::NoFactoryException::clone() const
{
    return new NoFactoryException(*this);
}

void
Ice::NoFactoryException::raise() const
{
    throw *this;
}

Ice::SystemException::SystemException(const char* file, int line) :
    LocalException(file, line)
{
#ifdef WIN32
    _error = WSAGetLastError();
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
	_error = ex._error;

    return *this;
}

string
Ice::SystemException::toString() const
{
    string s = debugInfo() + "system exception: ";
    s += errorToString(_error);
    return s;
}

LocalException*
Ice::SystemException::clone() const
{
    return new SystemException(*this);
}

void
Ice::SystemException::raise() const
{
    throw *this;
}

Ice::SocketException::SocketException(const char* file, int line) :
    SystemException(file, line)
{
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
Ice::SocketException::toString() const
{
    string s = debugInfo() + "socket exception: ";
    s += errorToString(_error);
    return s;
}

LocalException*
Ice::SocketException::clone() const
{
    return new SocketException(*this);
}

void
Ice::SocketException::raise() const
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
Ice::TimeoutException::toString() const
{
    return debugInfo() + "timeout while sending or receiving data";
}

LocalException*
Ice::TimeoutException::clone() const
{
    return new TimeoutException(*this);
}

void
Ice::TimeoutException::raise() const
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
Ice::ConnectTimeoutException::toString() const
{
    return debugInfo() + "timeout while establishing a connection";
}

LocalException*
Ice::ConnectTimeoutException::clone() const
{
    return new ConnectTimeoutException(*this);
}

void
Ice::ConnectTimeoutException::raise() const
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
Ice::ConnectFailedException::toString() const
{
    string s = debugInfo() + "connect failed: ";
    s += errorToString(_error);
    return s;
}

LocalException*
Ice::ConnectFailedException::clone() const
{
    return new ConnectFailedException(*this);
}

void
Ice::ConnectFailedException::raise() const
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
Ice::ConnectionLostException::toString() const
{
    string s = debugInfo() + "connection lost: ";
    if (_error == 0)
	s += "recv() returned zero";
    else
	s += errorToString(_error);
    return s;
}

LocalException*
Ice::ConnectionLostException::clone() const
{
    return new ConnectionLostException(*this);
}

void
Ice::ConnectionLostException::raise() const
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
Ice::DNSException::toString() const
{
    string s = debugInfo() + "DNS error: ";
    s += errorToStringDNS(_error);
    return s;
}

LocalException*
Ice::DNSException::clone() const
{
    return new DNSException(*this);
}

void
Ice::DNSException::raise() const
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
Ice::UnmarshalOutOfBoundsException::toString() const
{
    return debugInfo() + "protocol error: out of bounds during unmarshaling";
}

LocalException*
Ice::UnmarshalOutOfBoundsException::clone() const
{
    return new UnmarshalOutOfBoundsException(*this);
}

void
Ice::UnmarshalOutOfBoundsException::raise() const
{
    throw *this;
}

Ice::ValueUnmarshalException::ValueUnmarshalException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::ValueUnmarshalException::ValueUnmarshalException(const ValueUnmarshalException& ex) :
    ProtocolException(ex)
{
}

ValueUnmarshalException&
Ice::ValueUnmarshalException::operator=(const ValueUnmarshalException& ex)
{
    ProtocolException::operator=(ex);
    return *this;
}

string
Ice::ValueUnmarshalException::toString() const
{
    return debugInfo() + "protocol error: value type does not match signature";
}

LocalException*
Ice::ValueUnmarshalException::clone() const
{
    return new ValueUnmarshalException(*this);
}

void
Ice::ValueUnmarshalException::raise() const
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
Ice::StringEncodingException::toString() const
{
    return debugInfo() + "protocol error: string encoding error";
}

LocalException*
Ice::StringEncodingException::clone() const
{
    return new StringEncodingException(*this);
}

void
Ice::StringEncodingException::raise() const
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
Ice::MemoryLimitException::toString() const
{
    return debugInfo() + "protocol error: memory limit exceeded";
}

LocalException*
Ice::MemoryLimitException::clone() const
{
    return new MemoryLimitException(*this);
}

void
Ice::MemoryLimitException::raise() const
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
Ice::EncapsulationException::toString() const
{
    return debugInfo() + "protocol error: illegal encapsulation";
}

LocalException*
Ice::EncapsulationException::clone() const
{
    return new EncapsulationException(*this);
}

void
Ice::EncapsulationException::raise() const
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
Ice::UnsupportedProtocolException::toString() const
{
    return debugInfo() + "protocol error: unsupported protocol version";
}

LocalException*
Ice::UnsupportedProtocolException::clone() const
{
    return new UnsupportedProtocolException(*this);
}

void
Ice::UnsupportedProtocolException::raise() const
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
Ice::UnsupportedEncodingException::toString() const
{
    return debugInfo() + "protocol error: unsupported encoding version";
}

LocalException*
Ice::UnsupportedEncodingException::clone() const
{
    return new UnsupportedEncodingException(*this);
}

void
Ice::UnsupportedEncodingException::raise() const
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
Ice::InvalidMessageException::toString() const
{
    return debugInfo() + "protocol error: invalid message type";
}

LocalException*
Ice::InvalidMessageException::clone() const
{
    return new InvalidMessageException(*this);
}

void
Ice::InvalidMessageException::raise() const
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
Ice::UnknownMessageException::toString() const
{
    return debugInfo() + "protocol error: unknown message type";
}

LocalException*
Ice::UnknownMessageException::clone() const
{
    return new UnknownMessageException(*this);
}

void
Ice::UnknownMessageException::raise() const
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
Ice::UnknownRequestIdException::toString() const
{
    return debugInfo() + "protocol error: unknown request id";
}

LocalException*
Ice::UnknownRequestIdException::clone() const
{
    return new UnknownRequestIdException(*this);
}

void
Ice::UnknownRequestIdException::raise() const
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
Ice::UnknownReplyStatusException::toString() const
{
    return debugInfo() + "protocol error: unknown reply status";
}

LocalException*
Ice::UnknownReplyStatusException::clone() const
{
    return new UnknownReplyStatusException(*this);
}

void
Ice::UnknownReplyStatusException::raise() const
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
Ice::CloseConnectionException::toString() const
{
    return debugInfo() + "protocol error: connection closed by server";
}

LocalException*
Ice::CloseConnectionException::clone() const
{
    return new CloseConnectionException(*this);
}

void
Ice::CloseConnectionException::raise() const
{
    throw *this;
}

