//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Initialize.h"
#include "Ice/LocalException.h"
#include "Network.h"
#include "Ice/StringUtil.h"

#include <iomanip>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    inline string socketErrorToString(int error)
    {
        if (error == 0)
        {
            return "unknown error";
        }
        return IceInternal::errorToString(error);
    }
}

void
Ice::RequestFailedException::ice_print(ostream& out) const
{
    Exception::ice_print(out); // print file + line + what()
    if (ice_hasDefaultMessage() && !_id->name.empty())
    {
        // We assume a non-default message includes identity/facet/operation.
        out << "\nidentity: '" << identityToString(*_id, ToStringMode::Unicode) << "'";
        out << "\nfacet: " << *_facet;
        out << "\noperation: " << *_operation;
    }
}

const char*
Ice::ObjectNotExistException::ice_id() const noexcept
{
    return "::Ice::ObjectNotExistException";
}

const char*
Ice::FacetNotExistException::ice_id() const noexcept
{
    return "::Ice::FacetNotExistException";
}

const char*
Ice::OperationNotExistException::ice_id() const noexcept
{
    return "::Ice::OperationNotExistException";
}

const char*
Ice::UnknownException::ice_id() const noexcept
{
    return "::Ice::UnknownException";
}

const char*
Ice::UnknownLocalException::ice_id() const noexcept
{
    return "::Ice::UnknownLocalException";
}

const char*
Ice::UnknownUserException::ice_id() const noexcept
{
    return "::Ice::UnknownUserException";
}

//
// Below: not refactored yet
//
const char*
Ice::InitializationException::ice_id() const noexcept
{
    return "::Ice::InitializationException";
}

const char*
Ice::PluginInitializationException::ice_id() const noexcept
{
    return "::Ice::PluginInitializationException";
}

const char*
Ice::AlreadyRegisteredException::ice_id() const noexcept
{
    return "::Ice::AlreadyRegisteredException";
}

const char*
Ice::NotRegisteredException::ice_id() const noexcept
{
    return "::Ice::NotRegisteredException";
}

const char*
Ice::TwowayOnlyException::ice_id() const noexcept
{
    return "::Ice::TwowayOnlyException";
}

const char*
Ice::CloneNotImplementedException::ice_id() const noexcept
{
    return "::Ice::CloneNotImplementedException";
}

const char*
Ice::VersionMismatchException::ice_id() const noexcept
{
    return "::Ice::VersionMismatchException";
}

const char*
Ice::CommunicatorDestroyedException::ice_id() const noexcept
{
    return "::Ice::CommunicatorDestroyedException";
}

const char*
Ice::ObjectAdapterDeactivatedException::ice_id() const noexcept
{
    return "::Ice::ObjectAdapterDeactivatedException";
}

const char*
Ice::ObjectAdapterIdInUseException::ice_id() const noexcept
{
    return "::Ice::ObjectAdapterIdInUseException";
}

const char*
Ice::NoEndpointException::ice_id() const noexcept
{
    return "::Ice::NoEndpointException";
}

const char*
Ice::EndpointParseException::ice_id() const noexcept
{
    return "::Ice::EndpointParseException";
}

const char*
Ice::EndpointSelectionTypeParseException::ice_id() const noexcept
{
    return "::Ice::EndpointSelectionTypeParseException";
}

const char*
Ice::VersionParseException::ice_id() const noexcept
{
    return "::Ice::VersionParseException";
}

const char*
Ice::IdentityParseException::ice_id() const noexcept
{
    return "::Ice::IdentityParseException";
}

const char*
Ice::ProxyParseException::ice_id() const noexcept
{
    return "::Ice::ProxyParseException";
}

const char*
Ice::IllegalIdentityException::ice_id() const noexcept
{
    return "::Ice::IllegalIdentityException";
}

const char*
Ice::IllegalServantException::ice_id() const noexcept
{
    return "::Ice::IllegalServantException";
}

Ice::SyscallException::SyscallException(const char* file, int line) noexcept
    : LocalException(file, line),
#ifdef _WIN32
      error(GetLastError())
#else
      error(errno)
#endif
{
}

const char*
Ice::SyscallException::ice_id() const noexcept
{
    return "::Ice::SyscallException";
}

const char*
Ice::SocketException::ice_id() const noexcept
{
    return "::Ice::SocketException";
}

const char*
Ice::CFNetworkException::ice_id() const noexcept
{
    return "::Ice::CFNetworkException";
}

const char*
Ice::FileException::ice_id() const noexcept
{
    return "::Ice::FileException";
}

const char*
Ice::ConnectFailedException::ice_id() const noexcept
{
    return "::Ice::ConnectFailedException";
}

const char*
Ice::ConnectionRefusedException::ice_id() const noexcept
{
    return "::Ice::ConnectionRefusedException";
}

const char*
Ice::ConnectionLostException::ice_id() const noexcept
{
    return "::Ice::ConnectionLostException";
}

const char*
Ice::DNSException::ice_id() const noexcept
{
    return "::Ice::DNSException";
}

const char*
Ice::ConnectionClosedException::ice_id() const noexcept
{
    return "::Ice::ConnectionClosedException";
}

const char*
Ice::ConnectionIdleException::ice_id() const noexcept
{
    return "::Ice::ConnectionIdleException";
}

const char*
Ice::TimeoutException::ice_id() const noexcept
{
    return "::Ice::TimeoutException";
}

const char*
Ice::ConnectTimeoutException::ice_id() const noexcept
{
    return "::Ice::ConnectTimeoutException";
}

const char*
Ice::CloseTimeoutException::ice_id() const noexcept
{
    return "::Ice::CloseTimeoutException";
}

const char*
Ice::InvocationTimeoutException::ice_id() const noexcept
{
    return "::Ice::InvocationTimeoutException";
}

const char*
Ice::InvocationCanceledException::ice_id() const noexcept
{
    return "::Ice::InvocationCanceledException";
}

const char*
Ice::ProtocolException::ice_id() const noexcept
{
    return "::Ice::ProtocolException";
}

const char*
Ice::BadMagicException::ice_id() const noexcept
{
    return "::Ice::BadMagicException";
}

const char*
Ice::UnsupportedProtocolException::ice_id() const noexcept
{
    return "::Ice::UnsupportedProtocolException";
}

const char*
Ice::UnsupportedEncodingException::ice_id() const noexcept
{
    return "::Ice::UnsupportedEncodingException";
}

const char*
Ice::UnknownMessageException::ice_id() const noexcept
{
    return "::Ice::UnknownMessageException";
}

const char*
Ice::ConnectionNotValidatedException::ice_id() const noexcept
{
    return "::Ice::ConnectionNotValidatedException";
}

const char*
Ice::UnknownReplyStatusException::ice_id() const noexcept
{
    return "::Ice::UnknownReplyStatusException";
}

const char*
Ice::CloseConnectionException::ice_id() const noexcept
{
    return "::Ice::CloseConnectionException";
}

const char*
Ice::ConnectionManuallyClosedException::ice_id() const noexcept
{
    return "::Ice::ConnectionManuallyClosedException";
}

const char*
Ice::IllegalMessageSizeException::ice_id() const noexcept
{
    return "::Ice::IllegalMessageSizeException";
}

const char*
Ice::CompressionException::ice_id() const noexcept
{
    return "::Ice::CompressionException";
}

const char*
Ice::DatagramLimitException::ice_id() const noexcept
{
    return "::Ice::DatagramLimitException";
}

const char*
Ice::MarshalException::ice_id() const noexcept
{
    return "::Ice::MarshalException";
}

const char*
Ice::ProxyUnmarshalException::ice_id() const noexcept
{
    return "::Ice::ProxyUnmarshalException";
}

const char*
Ice::UnmarshalOutOfBoundsException::ice_id() const noexcept
{
    return "::Ice::UnmarshalOutOfBoundsException";
}

const char*
Ice::NoValueFactoryException::ice_id() const noexcept
{
    return "::Ice::NoValueFactoryException";
}

const char*
Ice::UnexpectedObjectException::ice_id() const noexcept
{
    return "::Ice::UnexpectedObjectException";
}

const char*
Ice::MemoryLimitException::ice_id() const noexcept
{
    return "::Ice::MemoryLimitException";
}

const char*
Ice::StringConversionException::ice_id() const noexcept
{
    return "::Ice::StringConversionException";
}

const char*
Ice::EncapsulationException::ice_id() const noexcept
{
    return "::Ice::EncapsulationException";
}

const char*
Ice::FeatureNotSupportedException::ice_id() const noexcept
{
    return "::Ice::FeatureNotSupportedException";
}

const char*
Ice::SecurityException::ice_id() const noexcept
{
    return "::Ice::SecurityException";
}

const char*
Ice::FixedProxyException::ice_id() const noexcept
{
    return "::Ice::FixedProxyException";
}

// ice_print for all exceptions

void
Ice::InitializationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninitialization exception";
    if (!reason.empty())
    {
        out << ":\n" << reason;
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
Ice::VersionParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing version `" << str << "'";
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
    out << ":\nan identity with an empty name is not allowed";
}

void
Ice::IllegalServantException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nillegal servant: `" << reason << "'";
}

void
Ice::SyscallException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    if (error != 0)
    {
        out << ":\nsyscall exception: " << IceInternal::errorToString(error);
    }
}

void
Ice::SocketException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsocket exception: " << socketErrorToString(error);
}

void
Ice::FileException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfile exception: ";
    if (error == 0)
    {
        out << "couldn't open file";
    }
    else
    {
        out << IceInternal::errorToString(error);
    }
    if (!path.empty())
    {
        out << "\npath: " << path;
    }
}

void
Ice::ConnectFailedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnect failed: " << socketErrorToString(error);
}

void
Ice::ConnectionRefusedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection refused: " << socketErrorToString(error);
}

void
Ice::ConnectionLostException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection lost: ";
    if (error == 0)
    {
        out << "recv() returned zero";
    }
    else
    {
        out << socketErrorToString(error);
    }
}

void
Ice::DNSException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nDNS error: ";
    out << errorToStringDNS(error);
    out << "\nhost: " << host;
}

void
Ice::ConnectionClosedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << message;
}

void
Ice::ConnectionIdleException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection aborted by the idle check";
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
Ice::InvocationTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninvocation has timed out";
}

void
Ice::InvocationCanceledException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninvocation canceled";
}

void
Ice::ProtocolException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol exception";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::BadMagicException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown magic number: ";

    ios_base::fmtflags originalFlags = out.flags(); // Save stream state
    ostream::char_type originalFill = out.fill();

    out.flags(ios_base::hex); // Change to hex
    out.fill('0');            // Fill with leading zeros

    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[0])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[1])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[2])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[3]));

    out.fill(originalFill); // Restore stream state
    out.flags(originalFlags);

    if (!reason.empty())
    {
        out << "\n" << reason;
    }
}

void
Ice::UnsupportedProtocolException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unsupported protocol version: " << bad;
    out << "\n(can only support protocols compatible with version " << supported << ")";
}

void
Ice::UnsupportedEncodingException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nencoding error: unsupported encoding version: " << bad;
    out << "\n(can only support encodings compatible with version " << supported << ")";
    if (!reason.empty())
    {
        out << "\n" << reason;
    }
}

void
Ice::UnknownMessageException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unknown message type";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ConnectionNotValidatedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: received message over unvalidated connection";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnknownReplyStatusException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unknown reply status";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::CloseConnectionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: connection closed";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ConnectionManuallyClosedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: connection manually closed (" << (graceful ? "gracefully" : "forcefully") << ")";
}

void
Ice::IllegalMessageSizeException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: illegal message size";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::CompressionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: failed to compress or uncompress data";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::DatagramLimitException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: maximum datagram payload size exceeded";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::MarshalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: error during marshaling or unmarshaling";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ProxyUnmarshalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: inconsistent proxy data during unmarshaling";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnmarshalOutOfBoundsException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: out of bounds during unmarshaling";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::NoValueFactoryException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: no suitable value factory found for `" << type << "'";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnexpectedObjectException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunexpected class instance of type `" << type << "'; expected instance of type `" << expectedType << "'";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::MemoryLimitException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: memory limit exceeded";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::StringConversionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: string conversion failed";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::EncapsulationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: illegal encapsulation";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::PluginInitializationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nplug-in initialization failed";
    if (!reason.empty())
    {
        out << ": " << reason;
    }
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
    out << ":\nno " << kindOfObject << " with id `" << id << "' is registered";
}

void
Ice::TwowayOnlyException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\noperation `" << operation << "' can only be invoked as a twoway request";
}

void
Ice::CloneNotImplementedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nice_clone() must be implemented in classes derived from abstract base classes";
}

void
Ice::FeatureNotSupportedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfeature `" << unsupportedFeature << "' is not supported";
}

void
Ice::SecurityException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsecurity exception";
    if (!reason.empty())
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
Ice::CFNetworkException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nnetwork exception: domain: " << domain << " error: " << error;
}

// TODO: move to another file

string
IceInternal::createRequestFailedMessage(
    const char* typeId,
    const Ice::Identity& id,
    const string& facet,
    const string& operation)
{
    ostringstream os;
    os << "dispatch failed with " << typeId;
    if (!id.name.empty())
    {
        os << "\nidentity: '" << identityToString(id, ToStringMode::Unicode) << "'";
        os << "\nfacet: " << facet;
        os << "\noperation: " << operation;
    }
    return os.str();
}
