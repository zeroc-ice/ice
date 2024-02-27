//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif
#include <Ice/LocalException.h>
#include <IceUtil/PushDisableWarnings.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
#include <IceUtil/PopDisableWarnings.h>

#if defined(_MSC_VER)
#   pragma warning(disable:4458) // declaration of ... hides class member
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

Ice::InitializationException::~InitializationException()
{
}

std::string_view
Ice::InitializationException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::InitializationException";
    return typeId;
}

Ice::PluginInitializationException::~PluginInitializationException()
{
}

std::string_view
Ice::PluginInitializationException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::PluginInitializationException";
    return typeId;
}

Ice::CollocationOptimizationException::~CollocationOptimizationException()
{
}

std::string_view
Ice::CollocationOptimizationException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::CollocationOptimizationException";
    return typeId;
}

Ice::AlreadyRegisteredException::~AlreadyRegisteredException()
{
}

std::string_view
Ice::AlreadyRegisteredException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::AlreadyRegisteredException";
    return typeId;
}

Ice::NotRegisteredException::~NotRegisteredException()
{
}

std::string_view
Ice::NotRegisteredException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::NotRegisteredException";
    return typeId;
}

Ice::TwowayOnlyException::~TwowayOnlyException()
{
}

std::string_view
Ice::TwowayOnlyException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::TwowayOnlyException";
    return typeId;
}

Ice::CloneNotImplementedException::~CloneNotImplementedException()
{
}

std::string_view
Ice::CloneNotImplementedException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::CloneNotImplementedException";
    return typeId;
}

Ice::UnknownException::~UnknownException()
{
}

std::string_view
Ice::UnknownException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::UnknownException";
    return typeId;
}

Ice::UnknownLocalException::~UnknownLocalException()
{
}

std::string_view
Ice::UnknownLocalException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::UnknownLocalException";
    return typeId;
}

Ice::UnknownUserException::~UnknownUserException()
{
}

std::string_view
Ice::UnknownUserException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::UnknownUserException";
    return typeId;
}

Ice::VersionMismatchException::~VersionMismatchException()
{
}

std::string_view
Ice::VersionMismatchException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::VersionMismatchException";
    return typeId;
}

Ice::CommunicatorDestroyedException::~CommunicatorDestroyedException()
{
}

std::string_view
Ice::CommunicatorDestroyedException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::CommunicatorDestroyedException";
    return typeId;
}

Ice::ObjectAdapterDeactivatedException::~ObjectAdapterDeactivatedException()
{
}

std::string_view
Ice::ObjectAdapterDeactivatedException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ObjectAdapterDeactivatedException";
    return typeId;
}

Ice::ObjectAdapterIdInUseException::~ObjectAdapterIdInUseException()
{
}

std::string_view
Ice::ObjectAdapterIdInUseException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ObjectAdapterIdInUseException";
    return typeId;
}

Ice::NoEndpointException::~NoEndpointException()
{
}

std::string_view
Ice::NoEndpointException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::NoEndpointException";
    return typeId;
}

Ice::EndpointParseException::~EndpointParseException()
{
}

std::string_view
Ice::EndpointParseException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::EndpointParseException";
    return typeId;
}

Ice::EndpointSelectionTypeParseException::~EndpointSelectionTypeParseException()
{
}

std::string_view
Ice::EndpointSelectionTypeParseException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::EndpointSelectionTypeParseException";
    return typeId;
}

Ice::VersionParseException::~VersionParseException()
{
}

std::string_view
Ice::VersionParseException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::VersionParseException";
    return typeId;
}

Ice::IdentityParseException::~IdentityParseException()
{
}

std::string_view
Ice::IdentityParseException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::IdentityParseException";
    return typeId;
}

Ice::ProxyParseException::~ProxyParseException()
{
}

std::string_view
Ice::ProxyParseException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ProxyParseException";
    return typeId;
}

Ice::IllegalIdentityException::~IllegalIdentityException()
{
}

std::string_view
Ice::IllegalIdentityException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::IllegalIdentityException";
    return typeId;
}

Ice::IllegalServantException::~IllegalServantException()
{
}

std::string_view
Ice::IllegalServantException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::IllegalServantException";
    return typeId;
}

Ice::RequestFailedException::~RequestFailedException()
{
}

std::string_view
Ice::RequestFailedException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::RequestFailedException";
    return typeId;
}

Ice::ObjectNotExistException::~ObjectNotExistException()
{
}

std::string_view
Ice::ObjectNotExistException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ObjectNotExistException";
    return typeId;
}

Ice::FacetNotExistException::~FacetNotExistException()
{
}

std::string_view
Ice::FacetNotExistException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::FacetNotExistException";
    return typeId;
}

Ice::OperationNotExistException::~OperationNotExistException()
{
}

std::string_view
Ice::OperationNotExistException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::OperationNotExistException";
    return typeId;
}

Ice::SyscallException::~SyscallException()
{
}

std::string_view
Ice::SyscallException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::SyscallException";
    return typeId;
}

Ice::SocketException::~SocketException()
{
}

std::string_view
Ice::SocketException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::SocketException";
    return typeId;
}

Ice::CFNetworkException::~CFNetworkException()
{
}

std::string_view
Ice::CFNetworkException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::CFNetworkException";
    return typeId;
}

Ice::FileException::~FileException()
{
}

std::string_view
Ice::FileException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::FileException";
    return typeId;
}

Ice::ConnectFailedException::~ConnectFailedException()
{
}

std::string_view
Ice::ConnectFailedException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ConnectFailedException";
    return typeId;
}

Ice::ConnectionRefusedException::~ConnectionRefusedException()
{
}

std::string_view
Ice::ConnectionRefusedException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ConnectionRefusedException";
    return typeId;
}

Ice::ConnectionLostException::~ConnectionLostException()
{
}

std::string_view
Ice::ConnectionLostException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ConnectionLostException";
    return typeId;
}

Ice::DNSException::~DNSException()
{
}

std::string_view
Ice::DNSException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::DNSException";
    return typeId;
}

Ice::OperationInterruptedException::~OperationInterruptedException()
{
}

std::string_view
Ice::OperationInterruptedException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::OperationInterruptedException";
    return typeId;
}

Ice::TimeoutException::~TimeoutException()
{
}

std::string_view
Ice::TimeoutException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::TimeoutException";
    return typeId;
}

Ice::ConnectTimeoutException::~ConnectTimeoutException()
{
}

std::string_view
Ice::ConnectTimeoutException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ConnectTimeoutException";
    return typeId;
}

Ice::CloseTimeoutException::~CloseTimeoutException()
{
}

std::string_view
Ice::CloseTimeoutException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::CloseTimeoutException";
    return typeId;
}

Ice::ConnectionTimeoutException::~ConnectionTimeoutException()
{
}

std::string_view
Ice::ConnectionTimeoutException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ConnectionTimeoutException";
    return typeId;
}

Ice::InvocationTimeoutException::~InvocationTimeoutException()
{
}

std::string_view
Ice::InvocationTimeoutException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::InvocationTimeoutException";
    return typeId;
}

Ice::InvocationCanceledException::~InvocationCanceledException()
{
}

std::string_view
Ice::InvocationCanceledException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::InvocationCanceledException";
    return typeId;
}

Ice::ProtocolException::~ProtocolException()
{
}

std::string_view
Ice::ProtocolException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ProtocolException";
    return typeId;
}

Ice::BadMagicException::~BadMagicException()
{
}

std::string_view
Ice::BadMagicException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::BadMagicException";
    return typeId;
}

Ice::UnsupportedProtocolException::~UnsupportedProtocolException()
{
}

std::string_view
Ice::UnsupportedProtocolException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::UnsupportedProtocolException";
    return typeId;
}

Ice::UnsupportedEncodingException::~UnsupportedEncodingException()
{
}

std::string_view
Ice::UnsupportedEncodingException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::UnsupportedEncodingException";
    return typeId;
}

Ice::UnknownMessageException::~UnknownMessageException()
{
}

std::string_view
Ice::UnknownMessageException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::UnknownMessageException";
    return typeId;
}

Ice::ConnectionNotValidatedException::~ConnectionNotValidatedException()
{
}

std::string_view
Ice::ConnectionNotValidatedException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ConnectionNotValidatedException";
    return typeId;
}

Ice::UnknownRequestIdException::~UnknownRequestIdException()
{
}

std::string_view
Ice::UnknownRequestIdException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::UnknownRequestIdException";
    return typeId;
}

Ice::UnknownReplyStatusException::~UnknownReplyStatusException()
{
}

std::string_view
Ice::UnknownReplyStatusException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::UnknownReplyStatusException";
    return typeId;
}

Ice::CloseConnectionException::~CloseConnectionException()
{
}

std::string_view
Ice::CloseConnectionException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::CloseConnectionException";
    return typeId;
}

Ice::ConnectionManuallyClosedException::~ConnectionManuallyClosedException()
{
}

std::string_view
Ice::ConnectionManuallyClosedException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ConnectionManuallyClosedException";
    return typeId;
}

Ice::IllegalMessageSizeException::~IllegalMessageSizeException()
{
}

std::string_view
Ice::IllegalMessageSizeException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::IllegalMessageSizeException";
    return typeId;
}

Ice::CompressionException::~CompressionException()
{
}

std::string_view
Ice::CompressionException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::CompressionException";
    return typeId;
}

Ice::DatagramLimitException::~DatagramLimitException()
{
}

std::string_view
Ice::DatagramLimitException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::DatagramLimitException";
    return typeId;
}

Ice::MarshalException::~MarshalException()
{
}

std::string_view
Ice::MarshalException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::MarshalException";
    return typeId;
}

Ice::ProxyUnmarshalException::~ProxyUnmarshalException()
{
}

std::string_view
Ice::ProxyUnmarshalException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ProxyUnmarshalException";
    return typeId;
}

Ice::UnmarshalOutOfBoundsException::~UnmarshalOutOfBoundsException()
{
}

std::string_view
Ice::UnmarshalOutOfBoundsException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::UnmarshalOutOfBoundsException";
    return typeId;
}

Ice::NoValueFactoryException::~NoValueFactoryException()
{
}

std::string_view
Ice::NoValueFactoryException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::NoValueFactoryException";
    return typeId;
}

Ice::UnexpectedObjectException::~UnexpectedObjectException()
{
}

std::string_view
Ice::UnexpectedObjectException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::UnexpectedObjectException";
    return typeId;
}

Ice::MemoryLimitException::~MemoryLimitException()
{
}

std::string_view
Ice::MemoryLimitException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::MemoryLimitException";
    return typeId;
}

Ice::StringConversionException::~StringConversionException()
{
}

std::string_view
Ice::StringConversionException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::StringConversionException";
    return typeId;
}

Ice::EncapsulationException::~EncapsulationException()
{
}

std::string_view
Ice::EncapsulationException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::EncapsulationException";
    return typeId;
}

Ice::FeatureNotSupportedException::~FeatureNotSupportedException()
{
}

std::string_view
Ice::FeatureNotSupportedException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::FeatureNotSupportedException";
    return typeId;
}

Ice::SecurityException::~SecurityException()
{
}

std::string_view
Ice::SecurityException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::SecurityException";
    return typeId;
}

Ice::FixedProxyException::~FixedProxyException()
{
}

std::string_view
Ice::FixedProxyException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::FixedProxyException";
    return typeId;
}

Ice::ResponseSentException::~ResponseSentException()
{
}

std::string_view
Ice::ResponseSentException::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::ResponseSentException";
    return typeId;
}
