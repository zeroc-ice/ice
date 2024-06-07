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

std::string_view
Ice::LocalException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::LocalException";
    return typeId;
}

string
Ice::InitializationException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::InitializationException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::InitializationException";
    return typeId;
}

string
Ice::PluginInitializationException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::PluginInitializationException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::PluginInitializationException";
    return typeId;
}

string
Ice::AlreadyRegisteredException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::AlreadyRegisteredException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::AlreadyRegisteredException";
    return typeId;
}

string
Ice::NotRegisteredException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::NotRegisteredException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::NotRegisteredException";
    return typeId;
}

string
Ice::TwowayOnlyException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::TwowayOnlyException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::TwowayOnlyException";
    return typeId;
}

string
Ice::CloneNotImplementedException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::CloneNotImplementedException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::CloneNotImplementedException";
    return typeId;
}

string
Ice::UnknownException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::UnknownException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::UnknownException";
    return typeId;
}

string
Ice::UnknownLocalException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::UnknownLocalException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::UnknownLocalException";
    return typeId;
}

string
Ice::UnknownUserException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::UnknownUserException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::UnknownUserException";
    return typeId;
}

string
Ice::VersionMismatchException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::VersionMismatchException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::VersionMismatchException";
    return typeId;
}

string
Ice::CommunicatorDestroyedException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::CommunicatorDestroyedException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::CommunicatorDestroyedException";
    return typeId;
}

string
Ice::ObjectAdapterDeactivatedException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ObjectAdapterDeactivatedException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ObjectAdapterDeactivatedException";
    return typeId;
}

string
Ice::ObjectAdapterIdInUseException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ObjectAdapterIdInUseException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ObjectAdapterIdInUseException";
    return typeId;
}

string
Ice::NoEndpointException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::NoEndpointException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::NoEndpointException";
    return typeId;
}

string
Ice::EndpointParseException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::EndpointParseException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::EndpointParseException";
    return typeId;
}

string
Ice::EndpointSelectionTypeParseException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::EndpointSelectionTypeParseException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::EndpointSelectionTypeParseException";
    return typeId;
}

string
Ice::VersionParseException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::VersionParseException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::VersionParseException";
    return typeId;
}

string
Ice::IdentityParseException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::IdentityParseException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::IdentityParseException";
    return typeId;
}

string
Ice::ProxyParseException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ProxyParseException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ProxyParseException";
    return typeId;
}

string
Ice::IllegalIdentityException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::IllegalIdentityException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::IllegalIdentityException";
    return typeId;
}

string
Ice::IllegalServantException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::IllegalServantException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::IllegalServantException";
    return typeId;
}

string
Ice::RequestFailedException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::RequestFailedException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::RequestFailedException";
    return typeId;
}

string
Ice::ObjectNotExistException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ObjectNotExistException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ObjectNotExistException";
    return typeId;
}

string
Ice::FacetNotExistException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::FacetNotExistException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::FacetNotExistException";
    return typeId;
}

string
Ice::OperationNotExistException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::OperationNotExistException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::OperationNotExistException";
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

string
Ice::SyscallException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::SyscallException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::SyscallException";
    return typeId;
}

string
Ice::SocketException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::SocketException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::SocketException";
    return typeId;
}

string
Ice::CFNetworkException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::CFNetworkException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::CFNetworkException";
    return typeId;
}

string
Ice::FileException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::FileException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::FileException";
    return typeId;
}

string
Ice::ConnectFailedException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ConnectFailedException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ConnectFailedException";
    return typeId;
}

string
Ice::ConnectionRefusedException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ConnectionRefusedException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ConnectionRefusedException";
    return typeId;
}

string
Ice::ConnectionLostException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ConnectionLostException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ConnectionLostException";
    return typeId;
}

string
Ice::DNSException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::DNSException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::DNSException";
    return typeId;
}

string
Ice::ConnectionClosedException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ConnectionClosedException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ConnectionClosedException";
    return typeId;
}

string
Ice::ConnectionIdleException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ConnectionIdleException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ConnectionIdleException";
    return typeId;
}

string
Ice::TimeoutException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::TimeoutException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::TimeoutException";
    return typeId;
}

string
Ice::ConnectTimeoutException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ConnectTimeoutException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ConnectTimeoutException";
    return typeId;
}

string
Ice::CloseTimeoutException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::CloseTimeoutException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::CloseTimeoutException";
    return typeId;
}

string
Ice::InvocationTimeoutException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::InvocationTimeoutException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::InvocationTimeoutException";
    return typeId;
}

string
Ice::InvocationCanceledException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::InvocationCanceledException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::InvocationCanceledException";
    return typeId;
}

string
Ice::ProtocolException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ProtocolException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ProtocolException";
    return typeId;
}

string
Ice::BadMagicException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::BadMagicException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::BadMagicException";
    return typeId;
}

string
Ice::UnsupportedProtocolException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::UnsupportedProtocolException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::UnsupportedProtocolException";
    return typeId;
}

string
Ice::UnsupportedEncodingException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::UnsupportedEncodingException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::UnsupportedEncodingException";
    return typeId;
}

string
Ice::UnknownMessageException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::UnknownMessageException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::UnknownMessageException";
    return typeId;
}

string
Ice::ConnectionNotValidatedException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ConnectionNotValidatedException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ConnectionNotValidatedException";
    return typeId;
}

string
Ice::UnknownReplyStatusException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::UnknownReplyStatusException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::UnknownReplyStatusException";
    return typeId;
}

string
Ice::CloseConnectionException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::CloseConnectionException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::CloseConnectionException";
    return typeId;
}

string
Ice::ConnectionManuallyClosedException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ConnectionManuallyClosedException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ConnectionManuallyClosedException";
    return typeId;
}

string
Ice::IllegalMessageSizeException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::IllegalMessageSizeException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::IllegalMessageSizeException";
    return typeId;
}

string
Ice::CompressionException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::CompressionException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::CompressionException";
    return typeId;
}

string
Ice::DatagramLimitException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::DatagramLimitException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::DatagramLimitException";
    return typeId;
}

string
Ice::MarshalException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::MarshalException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::MarshalException";
    return typeId;
}

string
Ice::ProxyUnmarshalException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::ProxyUnmarshalException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::ProxyUnmarshalException";
    return typeId;
}

string
Ice::UnmarshalOutOfBoundsException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::UnmarshalOutOfBoundsException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::UnmarshalOutOfBoundsException";
    return typeId;
}

string
Ice::NoValueFactoryException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::NoValueFactoryException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::NoValueFactoryException";
    return typeId;
}

string
Ice::UnexpectedObjectException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::UnexpectedObjectException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::UnexpectedObjectException";
    return typeId;
}

string
Ice::MemoryLimitException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::MemoryLimitException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::MemoryLimitException";
    return typeId;
}

string
Ice::StringConversionException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::StringConversionException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::StringConversionException";
    return typeId;
}

string
Ice::EncapsulationException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::EncapsulationException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::EncapsulationException";
    return typeId;
}

string
Ice::FeatureNotSupportedException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::FeatureNotSupportedException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::FeatureNotSupportedException";
    return typeId;
}

string
Ice::SecurityException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::SecurityException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::SecurityException";
    return typeId;
}

string
Ice::FixedProxyException::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::FixedProxyException::ice_staticId() noexcept
{
    static constexpr string_view typeId = "::Ice::FixedProxyException";
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
