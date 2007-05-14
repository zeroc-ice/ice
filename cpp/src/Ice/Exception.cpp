// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Exception.h>
#include <Ice/LocalException.h>
#include <Ice/Network.h>
#include <Ice/Plugin.h>
#include <IceUtil/StringUtil.h>
#include <iomanip>

using namespace std;
using namespace Ice;
using namespace IceInternal;

bool
Ice::UserException::__usesClasses() const
{
    return false;
}

Ice::LocalException::LocalException(const char* file, int line) :
    Exception(file, line)
{
}

Ice::LocalException::~LocalException() throw()
{
}

#if defined(__SUNPRO_CC)
ostream&
Ice::operator<<(ostream& out, const Ice::UserException& ex)
{
    ex.ice_print(out);
    return out;
}

ostream&
Ice::operator<<(ostream& out, const Ice::LocalException& ex)
{
    ex.ice_print(out);
    return out;
}
#endif

void
Ice::InitializationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninitialization exception";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnknownException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown exception";
    if(!unknown.empty())
    {
        out << ":\n" << unknown;
    }
}

void
Ice::UnknownLocalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown local exception";
    if(!unknown.empty())
    {
        out << ":\n" << unknown;
    }
}

void
Ice::UnknownUserException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown user exception";
    if(!unknown.empty())
    {
        out << ":\n" << unknown;
    }
}

void
Ice::VersionMismatchException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nIce library version mismatch";
}

void
Ice::CommunicatorDestroyedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ncommunicator object destroyed";
}

void
Ice::ObjectAdapterDeactivatedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nobject adapter `" << name << "' deactivated";
}

void
Ice::ObjectAdapterIdInUseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nobject adapter with id `" << id << "' is already in use";
}

void
Ice::NoEndpointException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nno suitable endpoint available for proxy `" << proxy << "'";
}

void
Ice::EndpointParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing endpoint `" << str << "'";
}

void
Ice::EndpointSelectionTypeParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing endpoint selection type `" << str << "'";
}

void
Ice::IdentityParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing identity `" << str << "'";
}

void
Ice::ProxyParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing proxy `" << str << "'";
}

void
Ice::IllegalIdentityException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nillegal identity: `";
    if(id.category.empty())
    {
        out << IceUtil::escapeString(id.name, "/");
    }
    else
    {
        out << IceUtil::escapeString(id.category, "/") << '/' << IceUtil::escapeString(id.name, "/");
    }
    out << "'";
}

static void
printFailedRequestData(ostream& out, const RequestFailedException& ex)
{
    out << ":\nidentity: `";
    if(ex.id.category.empty())
    {
        out << IceUtil::escapeString(ex.id.name, "/");
    }
    else
    {
        out << IceUtil::escapeString(ex.id.category, "/") << '/' << IceUtil::escapeString(ex.id.name, "/");
    }
    out << "'";
    out << "\nfacet: " << ex.facet;
    out << "\noperation: " << ex.operation;
}

void
Ice::RequestFailedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nrequest failed";
    printFailedRequestData(out, *this);
}

void
Ice::ObjectNotExistException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nobject does not exist";
    printFailedRequestData(out, *this);
}

void
Ice::FacetNotExistException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfacet does not exist";
    printFailedRequestData(out, *this);
}

void
Ice::OperationNotExistException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\noperation does not exist";
    printFailedRequestData(out, *this);
}

void
Ice::SyscallException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    if(error != 0)
    {
        out << ":\nsyscall exception: " << errorToString(error);
    }
}

void
Ice::SocketException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsocket exception: ";
    if(error == 0)
    {
        out << "unknown error";
    }
    else
    {
        out << errorToString(error);
    }
}

void
Ice::FileException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfile exception: ";
    if(error == 0)
    {
        out << "couldn't open file";
    }
    else
    {
        out << errorToString(error);
    }
    if(!path.empty())
    {
        out << "\npath: " << path;
    }
}

void
Ice::ConnectFailedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnect failed: " << errorToString(error);
}

void
Ice::ConnectionRefusedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection refused: " << errorToString(error);
}

void
Ice::ConnectionLostException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection lost: ";
    if(error == 0)
    {
        out << "recv() returned zero";
    }
    else
    {
        out << errorToString(error);
    }
}

void
Ice::DNSException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nDNS error: " << errorToStringDNS(error) << "\nhost: " << host;
}

void
Ice::TimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while sending or receiving data";
}

void
Ice::ConnectTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while establishing a connection";
}

void
Ice::CloseTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while closing a connection";
}

void
Ice::ConnectionTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection has timed out";
}

void
Ice::ProtocolException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol exception";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::BadMagicException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown magic number: ";

    ios_base::fmtflags originalFlags = out.flags();     // Save stream state
    ostream::char_type originalFill = out.fill();

    out.flags(ios_base::hex);                           // Change to hex
    out.fill('0');                                      // Fill with leading zeros

    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[0])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[1])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[2])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[3]));

    out.fill(originalFill);                             // Restore stream state
    out.flags(originalFlags);

    if(!reason.empty())
    {
        out << "\n" << reason;
    }
}

void
Ice::UnsupportedProtocolException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unsupported protocol version: " << badMajor << "." << badMinor;
    out << "\n(can only support protocols compatible with version " << major << "." << minor << ")";
}

void
Ice::UnsupportedEncodingException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unsupported encoding version: " << badMajor << "." << badMinor;
    out << "\n(can only support encodings compatible with version " << major << "." << major << ")";
    if(!reason.empty())
    {
        out << "\n" << reason;
    }
}

void
Ice::UnknownMessageException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unknown message type";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ConnectionNotValidatedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: received message over unvalidated connection";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnknownRequestIdException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unknown request id";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnknownReplyStatusException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unknown reply status";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::CloseConnectionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: connection closed";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ForcedCloseConnectionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: connection forcefully closed";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::IllegalMessageSizeException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: illegal message size";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::CompressionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: failed to compress or uncompress data";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::DatagramLimitException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: maximum datagram payload size exceeded";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::MarshalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: error during marshaling or unmarshaling";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ProxyUnmarshalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: inconsistent proxy data during unmarshaling";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnmarshalOutOfBoundsException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: out of bounds during unmarshaling";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::IllegalIndirectionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: encountered illegal protocol indirection";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::NoObjectFactoryException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: no suitable object factory found for `" << type << "'";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}       

void
Ice::UnexpectedObjectException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunexpected class instance of type `" << type << 
        "'; expected instance of type `" << expectedType << "'";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}       

void
Ice::MemoryLimitException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: memory limit exceeded";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::StringConversionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: string conversion failed";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::EncapsulationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: illegal encapsulation";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::NegativeSizeException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: negative size for sequence, dictionary, etc.";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::PluginInitializationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nplugin initialization failed";
    if(!reason.empty())
    {
        out << ": " << reason;
    }
}

void
Ice::CollocationOptimizationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nrequested feature not available with collocation optimization";
}

void
Ice::AlreadyRegisteredException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << kindOfObject << " with id `" << id << "' is already registered";
}

void
Ice::NotRegisteredException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n no " << kindOfObject << " with id `" << id << "' is registered";
}

void
Ice::TwowayOnlyException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n operation `" << operation << "' can only be invoked as a twoway request";
}

void
Ice::CloneNotImplementedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n ice_clone() must be implemented in classes derived from abstract base classes";
}

void
Ice::FeatureNotSupportedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfeature `" << unsupportedFeature << "' is not supported.";
}

void
Ice::SecurityException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsecurity exception";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::FixedProxyException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfixed proxy exception";
}

void
Ice::ResponseSentException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nresponse sent exception";
}
