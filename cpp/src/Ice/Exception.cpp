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

ostream&
IceUtil::printException(ostream& out, const UnknownLocalException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nunknown local exception";
}

ostream&
IceUtil::printException(ostream& out, const UnknownUserException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nunknown user exception";
}

ostream&
IceUtil::printException(ostream& out, const UnknownException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nunknown C++ exception";
}

ostream&
IceUtil::printException(ostream& out, const VersionMismatchException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nIce library version mismatch";
}

ostream&
IceUtil::printException(ostream& out, const CommunicatorDestroyedException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\ncommunicator object destroyed";
}

ostream&
IceUtil::printException(ostream& out, const ObjectAdapterDeactivatedException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nobject adapter deactivated";
}

ostream&
IceUtil::printException(ostream& out, const NoEndpointException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nno suitable endpoint available";
}

ostream&
IceUtil::printException(ostream& out, const EndpointParseException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nerror while parsing endpoint";
}

ostream&
IceUtil::printException(ostream& out, const ReferenceParseException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nerror while parsing reference";
}

ostream&
IceUtil::printException(ostream& out, const ReferenceIdentityException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nreference identity mismatch in location forward";
}

ostream&
IceUtil::printException(ostream& out, const ObjectNotExistException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nobject does not exist";
}

ostream&
IceUtil::printException(ostream& out, const OperationNotExistException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\noperation does not exist";
}

ostream&
IceUtil::printException(ostream& out, const SystemException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nsystem exception: " << errorToString(ex.error);
}

ostream&
IceUtil::printException(ostream& out, const SocketException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nsystem exception: " << errorToString(ex.error);
}

ostream&
IceUtil::printException(ostream& out, const ConnectFailedException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nconnect failed: " << errorToString(ex.error);
}

ostream&
IceUtil::printException(ostream& out, const ConnectionLostException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    out << ":\nconnection lost: ";
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
    return out << ":\nDNS error: " + errorToStringDNS(ex.error);
}

ostream&
IceUtil::printException(ostream& out, const TimeoutException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\ntimeout while sending or receiving data";
}

ostream&
IceUtil::printException(ostream& out, const ConnectTimeoutException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\ntimeout while establishing a connection";
}

ostream&
IceUtil::printException(ostream& out, const ProtocolException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nunknown protocol exception";
}

ostream&
IceUtil::printException(ostream& out, const UnmarshalOutOfBoundsException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: out of bounds during unmarshaling";
}

ostream&
IceUtil::printException(ostream& out, const NoServantFactoryException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: no servant factory found for servant with operations";
}

ostream&
IceUtil::printException(ostream& out, const ServantUnmarshalException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: servant type unknown or doesn't match signature";
}

ostream&
IceUtil::printException(ostream& out, const UserExceptionUnmarshalException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: exception type unknown or doesn't match signature";
}

ostream&
IceUtil::printException(ostream& out, const StringEncodingException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: string encoding error";
}

ostream&
IceUtil::printException(ostream& out, const MemoryLimitException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: memory limit exceeded";
}

ostream&
IceUtil::printException(ostream& out, const EncapsulationException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: illegal encapsulation";
}

ostream&
IceUtil::printException(ostream& out, const UnsupportedProtocolException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: unsupported protocol version";
}

ostream&
IceUtil::printException(ostream& out, const UnsupportedEncodingException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: unsupported encoding version";
}

ostream&
IceUtil::printException(ostream& out, const InvalidMessageException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: invalid message type";
}

ostream&
IceUtil::printException(ostream& out, const UnknownMessageException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: unknown message type";
}

ostream&
IceUtil::printException(ostream& out, const UnknownRequestIdException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: unknown request id";
}

ostream&
IceUtil::printException(ostream& out, const UnknownReplyStatusException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: unknown reply status";
}

ostream&
IceUtil::printException(ostream& out, const CloseConnectionException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: connection closed by server";
}

ostream&
IceUtil::printException(ostream& out, const AbortBatchRequestException& ex)
{
    IceUtil::printException(out, static_cast<const IceUtil::Exception&>(ex));
    return out << ":\nprotocol error: batch request was aborted";
}
