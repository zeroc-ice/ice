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

void
Ice::UnknownLocalException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nunknown local exception";
}

void
Ice::UnknownUserException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nunknown user exception";
}

void
Ice::UnknownException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nunknown C++ exception";
}

void
Ice::VersionMismatchException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nIce library version mismatch";
}

void
Ice::CommunicatorDestroyedException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\ncommunicator object destroyed";
}

void
Ice::ObjectAdapterDeactivatedException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nobject adapter deactivated";
}

void
Ice::NoEndpointException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nno suitable endpoint available";
}

void
Ice::EndpointParseException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nerror while parsing endpoint";
}

void
Ice::ReferenceParseException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nerror while parsing reference";
}

void
Ice::ReferenceIdentityException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nreference identity mismatch in location forward";
}

void
Ice::ObjectNotExistException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nobject does not exist";
}

void
Ice::FacetNotExistException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nfacet does not exist";
}

void
Ice::OperationNotExistException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\noperation does not exist";
}

void
Ice::SystemException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nsystem exception: " << errorToString(error);
}

void
Ice::SocketException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nsocket exception: " << errorToString(error);
}

void
Ice::ConnectFailedException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nconnect failed: " << errorToString(error);
}

void
Ice::ConnectionLostException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nconnection lost: ";
    if (error == 0)
    {
	out << "recv() returned zero";
    }
    else
    {
	out << errorToString(error);
    }
}

void
Ice::DNSException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nDNS error: " << errorToStringDNS(error);
}

void
Ice::TimeoutException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\ntimeout while sending or receiving data";
}

void
Ice::ConnectTimeoutException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\ntimeout while establishing a connection";
}

void
Ice::ProtocolException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nunknown protocol exception";
}

void
Ice::UnmarshalOutOfBoundsException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: out of bounds during unmarshaling";
}

void
Ice::NoObjectFactoryException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: no object factory found for servant with operations";
}

void
Ice::ServantUnmarshalException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: servant type unknown or doesn't match signature";
}

void
Ice::UserExceptionUnmarshalException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: exception type unknown or doesn't match signature";
}

void
Ice::ProxyUnmarshalException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: inconsistent proxy data during unmarshaling";
}

void
Ice::StringEncodingException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: string or wide string encoding error";
}

void
Ice::ObjectEncodingException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: object encoding error";
}

void
Ice::MemoryLimitException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: memory limit exceeded";
}

void
Ice::EncapsulationException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: illegal encapsulation";
}

void
Ice::UnsupportedProtocolException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: unsupported protocol version";
}

void
Ice::UnsupportedEncodingException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: unsupported encoding version";
}

void
Ice::InvalidMessageException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: invalid message type";
}

void
Ice::UnknownMessageException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: unknown message type";
}

void
Ice::UnknownRequestIdException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: unknown request id";
}

void
Ice::UnknownReplyStatusException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: unknown reply status";
}

void
Ice::CloseConnectionException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: connection closed by server";
}

void
Ice::AbortBatchRequestException::_print(ostream& out) const
{
    Exception::_print(out);
    out << ":\nprotocol error: batch request was aborted";
}
