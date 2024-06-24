//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/LocalException.h"
#include "Network.h"
#include "StringUtil.h"

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
        return IceUtilInternal::errorToString(error);
    }
}

const char*
Ice::LocalException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::LocalException";
    return typeId;
}

const char*
Ice::InitializationException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::InitializationException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::InitializationException";
    return typeId;
}

const char*
Ice::PluginInitializationException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::PluginInitializationException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::PluginInitializationException";
    return typeId;
}

const char*
Ice::AlreadyRegisteredException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::AlreadyRegisteredException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::AlreadyRegisteredException";
    return typeId;
}

const char*
Ice::NotRegisteredException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::NotRegisteredException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::NotRegisteredException";
    return typeId;
}

const char*
Ice::TwowayOnlyException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::TwowayOnlyException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::TwowayOnlyException";
    return typeId;
}

const char*
Ice::CloneNotImplementedException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::CloneNotImplementedException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::CloneNotImplementedException";
    return typeId;
}

const char*
Ice::UnknownException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::UnknownException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::UnknownException";
    return typeId;
}

const char*
Ice::UnknownLocalException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::UnknownLocalException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::UnknownLocalException";
    return typeId;
}

const char*
Ice::UnknownUserException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::UnknownUserException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::UnknownUserException";
    return typeId;
}

const char*
Ice::VersionMismatchException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::VersionMismatchException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::VersionMismatchException";
    return typeId;
}

const char*
Ice::CommunicatorDestroyedException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::CommunicatorDestroyedException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::CommunicatorDestroyedException";
    return typeId;
}

const char*
Ice::ObjectAdapterDeactivatedException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ObjectAdapterDeactivatedException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ObjectAdapterDeactivatedException";
    return typeId;
}

const char*
Ice::ObjectAdapterIdInUseException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ObjectAdapterIdInUseException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ObjectAdapterIdInUseException";
    return typeId;
}

const char*
Ice::NoEndpointException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::NoEndpointException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::NoEndpointException";
    return typeId;
}

const char*
Ice::EndpointParseException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::EndpointParseException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::EndpointParseException";
    return typeId;
}

const char*
Ice::EndpointSelectionTypeParseException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::EndpointSelectionTypeParseException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::EndpointSelectionTypeParseException";
    return typeId;
}

const char*
Ice::VersionParseException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::VersionParseException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::VersionParseException";
    return typeId;
}

const char*
Ice::IdentityParseException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::IdentityParseException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::IdentityParseException";
    return typeId;
}

const char*
Ice::ProxyParseException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ProxyParseException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ProxyParseException";
    return typeId;
}

const char*
Ice::IllegalIdentityException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::IllegalIdentityException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::IllegalIdentityException";
    return typeId;
}

const char*
Ice::IllegalServantException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::IllegalServantException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::IllegalServantException";
    return typeId;
}

const char*
Ice::RequestFailedException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::RequestFailedException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::RequestFailedException";
    return typeId;
}

const char*
Ice::ObjectNotExistException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ObjectNotExistException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ObjectNotExistException";
    return typeId;
}

const char*
Ice::FacetNotExistException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::FacetNotExistException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::FacetNotExistException";
    return typeId;
}

const char*
Ice::OperationNotExistException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::OperationNotExistException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::OperationNotExistException";
    return typeId;
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
Ice::SyscallException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::SyscallException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::SyscallException";
    return typeId;
}

const char*
Ice::SocketException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::SocketException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::SocketException";
    return typeId;
}

const char*
Ice::CFNetworkException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::CFNetworkException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::CFNetworkException";
    return typeId;
}

const char*
Ice::FileException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::FileException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::FileException";
    return typeId;
}

const char*
Ice::ConnectFailedException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ConnectFailedException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ConnectFailedException";
    return typeId;
}

const char*
Ice::ConnectionRefusedException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ConnectionRefusedException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ConnectionRefusedException";
    return typeId;
}

const char*
Ice::ConnectionLostException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ConnectionLostException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ConnectionLostException";
    return typeId;
}

const char*
Ice::DNSException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::DNSException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::DNSException";
    return typeId;
}

const char*
Ice::ConnectionClosedException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ConnectionClosedException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ConnectionClosedException";
    return typeId;
}

const char*
Ice::ConnectionIdleException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ConnectionIdleException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ConnectionIdleException";
    return typeId;
}

const char*
Ice::TimeoutException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::TimeoutException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::TimeoutException";
    return typeId;
}

const char*
Ice::ConnectTimeoutException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ConnectTimeoutException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ConnectTimeoutException";
    return typeId;
}

const char*
Ice::CloseTimeoutException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::CloseTimeoutException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::CloseTimeoutException";
    return typeId;
}

const char*
Ice::InvocationTimeoutException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::InvocationTimeoutException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::InvocationTimeoutException";
    return typeId;
}

const char*
Ice::InvocationCanceledException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::InvocationCanceledException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::InvocationCanceledException";
    return typeId;
}

const char*
Ice::ProtocolException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ProtocolException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ProtocolException";
    return typeId;
}

const char*
Ice::BadMagicException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::BadMagicException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::BadMagicException";
    return typeId;
}

const char*
Ice::UnsupportedProtocolException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::UnsupportedProtocolException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::UnsupportedProtocolException";
    return typeId;
}

const char*
Ice::UnsupportedEncodingException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::UnsupportedEncodingException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::UnsupportedEncodingException";
    return typeId;
}

const char*
Ice::UnknownMessageException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::UnknownMessageException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::UnknownMessageException";
    return typeId;
}

const char*
Ice::ConnectionNotValidatedException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ConnectionNotValidatedException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ConnectionNotValidatedException";
    return typeId;
}

const char*
Ice::UnknownReplyStatusException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::UnknownReplyStatusException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::UnknownReplyStatusException";
    return typeId;
}

const char*
Ice::CloseConnectionException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::CloseConnectionException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::CloseConnectionException";
    return typeId;
}

const char*
Ice::ConnectionManuallyClosedException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ConnectionManuallyClosedException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ConnectionManuallyClosedException";
    return typeId;
}

const char*
Ice::IllegalMessageSizeException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::IllegalMessageSizeException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::IllegalMessageSizeException";
    return typeId;
}

const char*
Ice::CompressionException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::CompressionException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::CompressionException";
    return typeId;
}

const char*
Ice::DatagramLimitException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::DatagramLimitException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::DatagramLimitException";
    return typeId;
}

const char*
Ice::MarshalException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::MarshalException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::MarshalException";
    return typeId;
}

const char*
Ice::ProxyUnmarshalException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::ProxyUnmarshalException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::ProxyUnmarshalException";
    return typeId;
}

const char*
Ice::UnmarshalOutOfBoundsException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::UnmarshalOutOfBoundsException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::UnmarshalOutOfBoundsException";
    return typeId;
}

const char*
Ice::NoValueFactoryException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::NoValueFactoryException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::NoValueFactoryException";
    return typeId;
}

const char*
Ice::UnexpectedObjectException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::UnexpectedObjectException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::UnexpectedObjectException";
    return typeId;
}

const char*
Ice::MemoryLimitException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::MemoryLimitException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::MemoryLimitException";
    return typeId;
}

const char*
Ice::StringConversionException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::StringConversionException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::StringConversionException";
    return typeId;
}

const char*
Ice::EncapsulationException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::EncapsulationException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::EncapsulationException";
    return typeId;
}

const char*
Ice::FeatureNotSupportedException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::FeatureNotSupportedException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::FeatureNotSupportedException";
    return typeId;
}

const char*
Ice::SecurityException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::SecurityException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::SecurityException";
    return typeId;
}

const char*
Ice::FixedProxyException::ice_id() const
{
    return ice_staticId();
}

const char*
Ice::FixedProxyException::ice_staticId() noexcept
{
    static constexpr const char* typeId = "::Ice::FixedProxyException";
    return typeId;
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
Ice::UnknownException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown exception";
    if (!unknown.empty())
    {
        out << ":\n" << unknown;
    }
}

void
Ice::UnknownLocalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown local exception";
    if (!unknown.empty())
    {
        out << ":\n" << unknown;
    }
}

void
Ice::UnknownUserException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown user exception";
    if (!unknown.empty())
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

static void
printFailedRequestData(ostream& out, const RequestFailedException& ex)
{
    out << ":\nidentity: `" << identityToString(ex.id, ToStringMode::Unicode) << "'";
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
    if (error != 0)
    {
        out << ":\nsyscall exception: " << IceUtilInternal::errorToString(error);
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
        out << IceUtilInternal::errorToString(error);
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
