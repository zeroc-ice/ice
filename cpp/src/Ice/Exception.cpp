// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Exception.h>
#include <Ice/Network.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::LocalException::LocalException(const char* file, int line) :
    Exception(file, line)
{
}

Ice::LocalException::LocalException(const LocalException& ex) :
    Exception(ex)
{
}

LocalException&
Ice::LocalException::operator=(const LocalException& ex)
{
    Exception::operator=(ex);
    return *this;
}

string
Ice::LocalException::_name() const
{
    return "Ice::LocalException";
}

ostream&
Ice::LocalException::_print(ostream& out) const
{
    return IceUtil::printException(out, *this);
}

Exception*
Ice::LocalException::_clone() const
{
    return new LocalException(*this);
}

void
Ice::LocalException::_throw() const
{
    throw *this;
}

Ice::UserException::UserException(const char* file, int line) :
    Exception(file, line)
{
}

Ice::UserException::UserException(const UserException& ex) :
    Exception(ex)
{
}

UserException&
Ice::UserException::operator=(const UserException& ex)
{
    Exception::operator=(ex);
    return *this;
}

string
Ice::UserException::_name() const
{
    return "Ice::UserException";
}

ostream&
Ice::UserException::_print(ostream& out) const
{
    return IceUtil::printException(out, *this);
}

Exception*
Ice::UserException::_clone() const
{
    return new UserException(*this);
}

void
Ice::UserException::_throw() const
{
    throw *this;
}

ostream&
IceUtil::printException(ostream& out, const LocalException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": unknown local exception";
}

ostream&
IceUtil::printException(ostream& out, const UserException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": unknown user exception";
}

ostream&
IceUtil::printException(ostream& out, const UnknownException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": unknown C++ exception";
}

ostream&
IceUtil::printException(ostream& out, const VersionMismatchException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": Ice library version mismatch";
}

ostream&
IceUtil::printException(ostream& out, const CommunicatorDestroyedException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": communicator object destroyed";
}

ostream&
IceUtil::printException(ostream& out, const ObjectAdapterDeactivatedException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": object adapter deactivated";
}

ostream&
IceUtil::printException(ostream& out, const NoEndpointException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": no suitable endpoint available";
}

ostream&
IceUtil::printException(ostream& out, const EndpointParseException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": error while parsing endpoint";
}

ostream&
IceUtil::printException(ostream& out, const ReferenceParseException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": error while parsing reference";
}

ostream&
IceUtil::printException(ostream& out, const ReferenceIdentityException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": reference identity mismatch in location forward";
}

ostream&
IceUtil::printException(ostream& out, const ObjectNotExistException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": object does not exist";
}

ostream&
IceUtil::printException(ostream& out, const OperationNotExistException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": operation does not exist";
}

ostream&
IceUtil::printException(ostream& out, const SystemException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": system exception: " << errorToString(ex.error);
}

ostream&
IceUtil::printException(ostream& out, const SocketException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": system exception: " << errorToString(ex.error);
}

ostream&
IceUtil::printException(ostream& out, const TimeoutException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": timeout while sending or receiving data";
}

ostream&
IceUtil::printException(ostream& out, const ConnectTimeoutException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": timeout while establishing a connection";
}

ostream&
IceUtil::printException(ostream& out, const ConnectFailedException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": connect failed: " << errorToString(ex.error);
}

ostream&
IceUtil::printException(ostream& out, const ConnectionLostException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    out << ": connection lost: ";
    if (ex.error == 0)
    {
	return out << "recv() returned zero";
    }
    else
    {
	return out << errorToString(ex.error);
    }
}

ostream&
IceUtil::printException(ostream& out, const DNSException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": DNS error: " + errorToStringDNS(ex.error);
}

ostream&
IceUtil::printException(ostream& out, const ProtocolException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": unknown protocol exception";
}

ostream&
IceUtil::printException(ostream& out, const UnmarshalOutOfBoundsException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: out of bounds during unmarshaling";
}

ostream&
IceUtil::printException(ostream& out, const ServantUnmarshalException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: no suitable servant factory available";
}

ostream&
IceUtil::printException(ostream& out, const StringEncodingException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: string encoding error";
}

ostream&
IceUtil::printException(ostream& out, const MemoryLimitException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: memory limit exceeded";
}

ostream&
IceUtil::printException(ostream& out, const EncapsulationException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: illegal encapsulation";
}

ostream&
IceUtil::printException(ostream& out, const UnsupportedProtocolException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: unsupported protocol version";
}

ostream&
IceUtil::printException(ostream& out, const UnsupportedEncodingException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: unsupported encoding version";
}

ostream&
IceUtil::printException(ostream& out, const InvalidMessageException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: invalid message type";
}

ostream&
IceUtil::printException(ostream& out, const UnknownMessageException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: unknown message type";
}

ostream&
IceUtil::printException(ostream& out, const UnknownRequestIdException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: unknown request id";
}

ostream&
IceUtil::printException(ostream& out, const UnknownReplyStatusException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: unknown reply status";
}

ostream&
IceUtil::printException(ostream& out, const CloseConnectionException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: connection closed by server";
}

ostream&
IceUtil::printException(ostream& out, const AbortBatchRequestException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ": protocol error: batch request was aborted";
}
