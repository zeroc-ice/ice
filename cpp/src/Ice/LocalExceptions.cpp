// Copyright (c) ZeroC, Inc.

#include "Ice/LocalExceptions.h"
#include "Ice/Initialize.h"
#include "Ice/Proxy.h"
#include "Ice/StringUtil.h"
#include "Network.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

//
// The 7 (8 with the RequestFailedException base class) special local exceptions that can be marshaled in an Ice
// reply message. Other local exceptions can't be marshaled.
//

namespace
{
    string createDispatchExceptionMessage(ReplyStatus replyStatus)
    {
        ostringstream os;
        os << "dispatch failed with reply status " << replyStatus;
        return os.str();
    }

    string createRequestFailedMessage(
        Ice::ReplyStatus replyStatus,
        const Identity& id,
        const string& facet,
        const string& operation)
    {
        std::ostringstream os;
        os << "dispatch failed with " << replyStatus;
        if (!id.name.empty())
        {
            os << "\nidentity: '" << identityToString(id, ToStringMode::Unicode) << "'";
            os << "\nfacet: " << facet;
            os << "\noperation: " << operation;
        }
        return os.str();
    }
}

Ice::DispatchException::DispatchException(const char* file, int line, ReplyStatus replyStatus, string message)
    : LocalException(file, line, std::move(message)),
      _replyStatus{replyStatus}
{
    if (_replyStatus <= ReplyStatus::UserException)
    {
        throw std::invalid_argument{
            "the replyStatus of a DispatchException must be greater than ReplyStatus::UserException"};
    }
}

Ice::DispatchException::DispatchException(const char* file, int line, ReplyStatus replyStatus)
    : DispatchException(file, line, replyStatus, createDispatchExceptionMessage(replyStatus))
{
}

const char*
Ice::DispatchException::ice_id() const noexcept
{
    return "::Ice::DispatchException";
}

Ice::RequestFailedException::~RequestFailedException() = default; // avoid weak vtable.

Ice::RequestFailedException::RequestFailedException(
    const char* file,
    int line,
    ReplyStatus replyStatus,
    Identity id,
    string facet,
    string operation)
    : DispatchException(file, line, replyStatus, createRequestFailedMessage(replyStatus, id, facet, operation)),
      _id{make_shared<Identity>(std::move(id))},
      _facet{make_shared<string>(std::move(facet))},
      _operation{make_shared<string>(std::move(operation))}
{
}

Ice::RequestFailedException::RequestFailedException(const char* file, int line, ReplyStatus replyStatus)
    : DispatchException(file, line, replyStatus),
      _id{make_shared<Identity>()},
      _facet{make_shared<string>()},
      _operation{make_shared<string>()}
{
}

Ice::ObjectNotExistException::ObjectNotExistException(
    const char* file,
    int line,
    Identity id,
    string facet,
    string operation)
    : RequestFailedException(
          file,
          line,
          ReplyStatus::ObjectNotExist,
          std::move(id),
          std::move(facet),
          std::move(operation))
{
}

Ice::ObjectNotExistException::ObjectNotExistException(const char* file, int line)
    : RequestFailedException(file, line, ReplyStatus::ObjectNotExist)
{
}

const char*
Ice::ObjectNotExistException::ice_id() const noexcept
{
    return "::Ice::ObjectNotExistException";
}

Ice::FacetNotExistException::FacetNotExistException(
    const char* file,
    int line,
    Identity id,
    string facet,
    string operation)
    : RequestFailedException(
          file,
          line,
          ReplyStatus::FacetNotExist,
          std::move(id),
          std::move(facet),
          std::move(operation))
{
}

Ice::FacetNotExistException::FacetNotExistException(const char* file, int line)
    : RequestFailedException(file, line, ReplyStatus::FacetNotExist)
{
}

const char*
Ice::FacetNotExistException::ice_id() const noexcept
{
    return "::Ice::FacetNotExistException";
}

Ice::OperationNotExistException::OperationNotExistException(
    const char* file,
    int line,
    Identity id,
    string facet,
    string operation)
    : RequestFailedException(
          file,
          line,
          ReplyStatus::OperationNotExist,
          std::move(id),
          std::move(facet),
          std::move(operation))
{
}

Ice::OperationNotExistException::OperationNotExistException(const char* file, int line)
    : RequestFailedException(file, line, ReplyStatus::OperationNotExist)
{
}

const char*
Ice::OperationNotExistException::ice_id() const noexcept
{
    return "::Ice::OperationNotExistException";
}

Ice::UnknownException::UnknownException(const char* file, int line, string message)
    : DispatchException{file, line, ReplyStatus::UnknownException, std::move(message)}
{
}

const char*
Ice::UnknownException::ice_id() const noexcept
{
    return "::Ice::UnknownException";
}

Ice::UnknownException::UnknownException(const char* file, int line, ReplyStatus replyStatus, string message)
    : DispatchException{file, line, replyStatus, std::move(message)}
{
}

Ice::UnknownLocalException::UnknownLocalException(const char* file, int line, string message)
    : UnknownException{file, line, ReplyStatus::UnknownLocalException, std::move(message)}
{
}

const char*
Ice::UnknownLocalException::ice_id() const noexcept
{
    return "::Ice::UnknownLocalException";
}

Ice::UnknownUserException::UnknownUserException(const char* file, int line, string message)
    : UnknownException{file, line, ReplyStatus::UnknownUserException, std::move(message)}
{
}

UnknownUserException
Ice::UnknownUserException::fromTypeId(const char* file, int line, const char* typeId)
{
    ostringstream os;
    os << "the reply carries a user exception that does not conform to the operation's exception specification: ";
    os << typeId;
    return UnknownUserException{file, line, os.str()};
}

const char*
Ice::UnknownUserException::ice_id() const noexcept
{
    return "::Ice::UnknownUserException";
}

//
// Protocol exceptions
//

const char*
Ice::ProtocolException::ice_id() const noexcept
{
    return "::Ice::ProtocolException";
}

const char*
Ice::CloseConnectionException::ice_id() const noexcept
{
    return "::Ice::CloseConnectionException";
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

//
// Timeout exceptions
//

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

//
// Syscall exceptions
//

Ice::SyscallException::SyscallException(
    const char* file,
    int line,
    string messagePrefix,
    SyscallException::ErrorCode error)
    : SyscallException(file, line, std::move(messagePrefix), error, IceInternal::errorToString)
{
}

Ice::SyscallException::SyscallException(
    const char* file,
    int line,
    string messagePrefix,
    ErrorCode error,
    const std::function<string(ErrorCode)>& errorToString)
    : LocalException(file, line, std::move(messagePrefix) + ": " + errorToString(error)),
      _error{error}
{
}

const char*
Ice::SyscallException::ice_id() const noexcept
{
    return "::Ice::SyscallException";
}

Ice::DNSException::DNSException(const char* file, int line, ErrorCode error, string_view host)
    : SyscallException(file, line, "cannot resolve DNS host '" + string{host} + "'", error, errorToStringDNS)
{
}

const char*
Ice::DNSException::ice_id() const noexcept
{
    return "::Ice::DNSException";
}

namespace
{
    string fileErrorToString(int error)
    {
        if (error == 0)
        {
            return "couldn't open file";
        }
        else
        {
            return IceInternal::errorToString(error);
        }
    }
}

Ice::FileException::FileException(const char* file, int line, string_view path, ErrorCode error)
    : SyscallException(file, line, "error while accessing file '" + string{path} + "'", error, fileErrorToString)
{
}

const char*
Ice::FileException::ice_id() const noexcept
{
    return "::Ice::FileException";
}

//
// Socket exceptions
//

namespace
{
    string socketErrorToString(ErrorCode error)
    {
        if (error == 0)
        {
            return "unknown error";
        }
        return IceInternal::errorToString(error);
    }
}

Ice::SocketException::SocketException(const char* file, int line, string messagePrefix, ErrorCode error)
    : SyscallException(file, line, std::move(messagePrefix), error, socketErrorToString)
{
}

const char*
Ice::SocketException::ice_id() const noexcept
{
    return "::Ice::SocketException";
}

const char*
Ice::ConnectFailedException::ice_id() const noexcept
{
    return "::Ice::ConnectFailedException";
}
namespace
{
    inline string connectionLostErrorToString(ErrorCode error)
    {
        if (error == 0)
        {
            return "recv() returned zero";
        }
        else
        {
            return socketErrorToString(error);
        }
    }
}

Ice::ConnectionLostException::ConnectionLostException(const char* file, int line, ErrorCode error)
    : SocketException(file, line, "connection lost", error, connectionLostErrorToString)
{
}

const char*
Ice::ConnectionLostException::ice_id() const noexcept
{
    return "::Ice::ConnectionLostException";
}

const char*
Ice::ConnectionRefusedException::ice_id() const noexcept
{
    return "::Ice::ConnectionRefusedException";
}

//
// Other leaf local exceptions
//

Ice::AlreadyRegisteredException::AlreadyRegisteredException(const char* file, int line, string kindOfObject, string id)
    : LocalException(file, line, "another " + kindOfObject + " is already registered with ID '" + id + "'"),
      _kindOfObject{make_shared<string>(std::move(kindOfObject))},
      _id{make_shared<string>(std::move(id))}
{
}

const char*
Ice::AlreadyRegisteredException::ice_id() const noexcept
{
    return "::Ice::AlreadyRegisteredException";
}

const char*
Ice::CommunicatorDestroyedException::ice_id() const noexcept
{
    return "::Ice::CommunicatorDestroyedException";
}

const char*
Ice::ConnectionAbortedException::ice_id() const noexcept
{
    return "::Ice::ConnectionAbortedException";
}

const char*
Ice::ConnectionClosedException::ice_id() const noexcept
{
    return "::Ice::ConnectionClosedException";
}

const char*
Ice::FeatureNotSupportedException::ice_id() const noexcept
{
    return "::Ice::FeatureNotSupportedException";
}

const char*
Ice::FixedProxyException::ice_id() const noexcept
{
    return "::Ice::FixedProxyException";
}

const char*
Ice::InitializationException::ice_id() const noexcept
{
    return "::Ice::InitializationException";
}

const char*
Ice::InvocationCanceledException::ice_id() const noexcept
{
    return "::Ice::InvocationCanceledException";
}

Ice::NoEndpointException::NoEndpointException(const char* file, int line, const ObjectPrx& proxy)
    : LocalException(file, line, "no suitable endpoint available for proxy '" + proxy.ice_toString() + "'")
{
}

const char*
Ice::NoEndpointException::ice_id() const noexcept
{
    return "::Ice::NoEndpointException";
}

Ice::NotRegisteredException::NotRegisteredException(const char* file, int line, string kindOfObject, string id)
    : LocalException(file, line, "no " + kindOfObject + " is registered with ID '" + id + "'"),
      _kindOfObject{make_shared<string>(std::move(kindOfObject))},
      _id{make_shared<string>(std::move(id))}
{
}

const char*
Ice::NotRegisteredException::ice_id() const noexcept
{
    return "::Ice::NotRegisteredException";
}

const char*
Ice::ObjectAdapterDeactivatedException::ice_id() const noexcept
{
    return "::Ice::ObjectAdapterDeactivatedException";
}

const char*
Ice::ObjectAdapterDestroyedException::ice_id() const noexcept
{
    return "::Ice::ObjectAdapterDestroyedException";
}

const char*
Ice::ObjectAdapterIdInUseException::ice_id() const noexcept
{
    return "::Ice::ObjectAdapterIdInUseException";
}

const char*
Ice::ParseException::ice_id() const noexcept
{
    return "::Ice::ParseException";
}

const char*
Ice::PluginInitializationException::ice_id() const noexcept
{
    return "::Ice::PluginInitializationException";
}

const char*
Ice::SecurityException::ice_id() const noexcept
{
    return "::Ice::SecurityException";
}

const char*
Ice::TwowayOnlyException::ice_id() const noexcept
{
    return "::Ice::TwowayOnlyException";
}

const char*
Ice::PropertyException::ice_id() const noexcept
{
    return "::Ice::PropertyException";
}
