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
Ice::UnknownLocalException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nunknown local exception";
}

void
Ice::UnknownUserException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nunknown user exception";
}

void
Ice::UnknownException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nunknown C++ exception";
}

void
Ice::VersionMismatchException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nIce library version mismatch";
}

void
Ice::CommunicatorDestroyedException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\ncommunicator object destroyed";
}

void
Ice::ObjectAdapterDeactivatedException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nobject adapter deactivated";
}

void
Ice::NoEndpointException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nno suitable endpoint available";
}

void
Ice::EndpointParseException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nerror while parsing endpoint";
}

void
Ice::ReferenceParseException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nerror while parsing reference";
}

void
Ice::ReferenceIdentityException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nreference identity mismatch in location forward";
}

void
Ice::ObjectNotExistException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nobject does not exist";
}

void
Ice::FacetNotExistException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nfacet does not exist";
}

void
Ice::OperationNotExistException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\noperation does not exist";
}

void
Ice::SystemException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nsystem exception: " << errorToString(error);
}

void
Ice::SocketException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nsocket exception: " << errorToString(error);
}

void
Ice::ConnectFailedException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nconnect failed: " << errorToString(error);
}

void
Ice::ConnectionLostException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
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
Ice::DNSException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nDNS error: " << errorToStringDNS(error);
}

void
Ice::TimeoutException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\ntimeout while sending or receiving data";
}

void
Ice::ConnectTimeoutException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\ntimeout while establishing a connection";
}

void
Ice::ProtocolException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nunknown protocol exception";
}

void
Ice::UnmarshalOutOfBoundsException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: out of bounds during unmarshaling";
}

void
Ice::NoObjectFactoryException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: no suitable object factory found";
}

void
Ice::NoUserExceptionFactoryException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: no suitable user exception factory found";
}

void
Ice::ProxyUnmarshalException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: inconsistent proxy data during unmarshaling";
}

void
Ice::IllegalIndirectionException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: encountered illegal protocol indirection";
}

void
Ice::MemoryLimitException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: memory limit exceeded";
}

void
Ice::EncapsulationException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: illegal encapsulation";
}

void
Ice::UnsupportedProtocolException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: unsupported protocol version";
}

void
Ice::UnsupportedEncodingException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: unsupported encoding version";
}

void
Ice::InvalidMessageException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: invalid message type";
}

void
Ice::UnknownMessageException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: unknown message type";
}

void
Ice::UnknownRequestIdException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: unknown request id";
}

void
Ice::UnknownReplyStatusException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: unknown reply status";
}

void
Ice::CloseConnectionException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: connection closed by server";
}

void
Ice::AbortBatchRequestException::_ice_print(ostream& out) const
{
    Exception::_ice_print(out);
    out << ":\nprotocol error: batch request was aborted";
}
